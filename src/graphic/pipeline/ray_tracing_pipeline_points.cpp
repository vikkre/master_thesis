#include "ray_tracing_pipeline_points.h"

#include "../helper/function_load.h"
#include "../device.h"
#include "../graphics_object.h"


#define GLOBAL_BINDING_SET_INDEX 0

#define RGEN_SHADER "raygen_points.spv"
#define RCHIT_SHADER "closesthit.spv"
#define RMISS_SHADER "miss.spv"
#define RSHADOW_SHADER "shadow.spv"


RayTracingPipelinePoints::RayTracingPipelinePoints(Device* device)
:Pipeline(device),
objects(), globalData(), raysToSend(10000),
rayTracingPipelineProperties(),
tlas(device),
raygenShaderBindingTable(device), missShaderBindingTable(device), hitShaderBindingTable(device),
storageImages(), shaderGroups(),
pipeline(VK_NULL_HANDLE), pipelineLayout(VK_NULL_HANDLE), descriptorPool(VK_NULL_HANDLE), globalDataBuffers(), descriptorSets(),
rgenCode(), rmissCode(), rchitCode() {}

RayTracingPipelinePoints::~RayTracingPipelinePoints() {
	vkDestroyDescriptorPool(device->getDevice(), descriptorPool, nullptr);
	vkDestroyPipeline(device->getDevice(), pipeline, nullptr);
	vkDestroyPipelineLayout(device->getDevice(), pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(device->getDevice(), device->renderInfo.globalDescriptorSetLayout, nullptr);
}

void RayTracingPipelinePoints::init() {
	rgenCode    = readFile(RGEN_SHADER   );
	rchitCode   = readFile(RCHIT_SHADER  );
	rmissCode   = readFile(RMISS_SHADER  );
	rshadowCode = readFile(RSHADOW_SHADER);

	getProperties();

	createDescriptorSetLayout(RayTracingPipelinePoints::getUniformBindings(), &device->renderInfo.globalDescriptorSetLayout);
	createStorageImages();
	createDescriptorPool();
	createBuffers();
	createDescriptorSets();
	createRayTracingPipeline();
	createShaderBindingTable();
}

void RayTracingPipelinePoints::getProperties() {
	rayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
	VkPhysicalDeviceProperties2 deviceProperties2{};
	deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	deviceProperties2.pNext = &rayTracingPipelineProperties;
	vkGetPhysicalDeviceProperties2(device->getPhysicalDevice(), &deviceProperties2);
}

void RayTracingPipelinePoints::recordPreRenderCommandBuffer(size_t index, const VkCommandBuffer* commandBuffer) {
	const uint32_t handleSizeAligned = alignedSize(rayTracingPipelineProperties.shaderGroupHandleSize, rayTracingPipelineProperties.shaderGroupHandleAlignment);

	VkStridedDeviceAddressRegionKHR raygenShaderSbtEntry{};
	raygenShaderSbtEntry.deviceAddress = raygenShaderBindingTable.getAddress();
	raygenShaderSbtEntry.stride = handleSizeAligned;
	raygenShaderSbtEntry.size = handleSizeAligned;

	VkStridedDeviceAddressRegionKHR missShaderSbtEntry{};
	missShaderSbtEntry.deviceAddress = missShaderBindingTable.getAddress();
	missShaderSbtEntry.stride = handleSizeAligned;
	missShaderSbtEntry.size = handleSizeAligned;

	VkStridedDeviceAddressRegionKHR hitShaderSbtEntry{};
	hitShaderSbtEntry.deviceAddress = hitShaderBindingTable.getAddress();
	hitShaderSbtEntry.stride = handleSizeAligned;
	hitShaderSbtEntry.size = handleSizeAligned;

	VkStridedDeviceAddressRegionKHR callableShaderSbtEntry{};

	vkCmdBindPipeline(*commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline);
	vkCmdBindDescriptorSets(
		*commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
		pipelineLayout,
		GLOBAL_BINDING_SET_INDEX, 1, &descriptorSets.at(index),
		0, nullptr
	);

	FuncLoad::vkCmdTraceRaysKHR(
		*commandBuffer,
		&raygenShaderSbtEntry,
		&missShaderSbtEntry,
		&hitShaderSbtEntry,
		&callableShaderSbtEntry,
		raysToSend, 1, 1
	);

	storageImages.at(index).copyImage(commandBuffer, device->renderInfo.swapchainImages.at(index));
}

void RayTracingPipelinePoints::updateUniforms(size_t index) {
	globalData.backgroundColor = device->renderInfo.backgroundColor;
	globalData.lightPosition = device->renderInfo.lightPosition;

	globalDataBuffers.at(index).passData((void*) &globalData);

	uint32_t count = 0;
	countBuffers.at(index).getData((void*) &count);
	std::cout << "Hello count: " << count << std::endl;

	count = 0;
	countBuffers.at(index).passData((void*) &count);

	for (size_t i = 0; i < objects.size(); ++i) {
		objects.at(i)->passBufferData(index);
		rtDataBuffers.at(index).passData(rtDataPtrs.at(i), i * GraphicsObject::getRTDataSize(), GraphicsObject::getRTDataSize());
	}
}

void RayTracingPipelinePoints::initTlas() {
	for (GraphicsObject* obj: objects) {
		GraphicsObject::ObjectInfo info = obj->getObjectInfo();
		tlas.blasInstances.push_back(info.instance);
		rtDataPtrs.push_back(info.dataPtr);
	}

	tlas.init();
}

void RayTracingPipelinePoints::createStorageImages() {
	storageImages.reserve(device->renderInfo.swapchainImageCount);

	for (size_t i = 0; i < device->renderInfo.swapchainImageCount; ++i) {
		storageImages.emplace_back(device);

		storageImages.at(i).width = device->renderInfo.swapchainExtend.width;
		storageImages.at(i).height = device->renderInfo.swapchainExtend.height;
		storageImages.at(i).format = device->renderInfo.swapchainImageFormat;
		storageImages.at(i).tiling = VK_IMAGE_TILING_OPTIMAL;
		storageImages.at(i).usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
		storageImages.at(i).properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		storageImages.at(i).aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
		storageImages.at(i).layout = VK_IMAGE_LAYOUT_GENERAL;

		storageImages.at(i).init();
		storageImages.at(i).createImageView();
	}
}

void RayTracingPipelinePoints::createShaderBindingTable() {
	const uint32_t handleSize = rayTracingPipelineProperties.shaderGroupHandleSize;
	const uint32_t handleSizeAligned = alignedSize(rayTracingPipelineProperties.shaderGroupHandleSize, rayTracingPipelineProperties.shaderGroupHandleAlignment);
	const uint32_t groupCount = shaderGroups.size();
	const uint32_t sbtSize = groupCount * handleSizeAligned;

	std::vector<uint8_t> shaderHandleStorage(sbtSize);
	if(FuncLoad::vkGetRayTracingShaderGroupHandlesKHR(device->getDevice(), pipeline, 0, groupCount, sbtSize, shaderHandleStorage.data()) != VK_SUCCESS) {
		throw InitException("vkGetRayTracingShaderGroupHandlesKHR", "could not get shader group handles");
	};

	const VkBufferUsageFlags bufferUsageFlags = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	const VkMemoryPropertyFlags memoryUsageFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	
	raygenShaderBindingTable.bufferSize = handleSize * 1;
	raygenShaderBindingTable.usage      = bufferUsageFlags;
	raygenShaderBindingTable.properties = memoryUsageFlags;
	raygenShaderBindingTable.init();
	
	missShaderBindingTable.bufferSize = handleSize * 2;
	missShaderBindingTable.usage      = bufferUsageFlags;
	missShaderBindingTable.properties = memoryUsageFlags;
	missShaderBindingTable.init();
	
	hitShaderBindingTable.bufferSize = handleSize * 1;
	hitShaderBindingTable.usage      = bufferUsageFlags;
	hitShaderBindingTable.properties = memoryUsageFlags;
	hitShaderBindingTable.init();

	raygenShaderBindingTable.passData((void*) &shaderHandleStorage[0 * handleSizeAligned]);
	missShaderBindingTable.passData  ((void*) &shaderHandleStorage[1 * handleSizeAligned]);
	hitShaderBindingTable.passData   ((void*) &shaderHandleStorage[3 * handleSizeAligned]);
}

void RayTracingPipelinePoints::createDescriptorPool() {
	VkDescriptorPoolSize tlasPoolSize{};
	tlasPoolSize.type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
	tlasPoolSize.descriptorCount = device->renderInfo.swapchainImageCount;

	VkDescriptorPoolSize imagePoolSize{};
	imagePoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	imagePoolSize.descriptorCount = device->renderInfo.swapchainImageCount;

	VkDescriptorPoolSize dataPoolSize{};
	dataPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	dataPoolSize.descriptorCount = device->renderInfo.swapchainImageCount;

	VkDescriptorPoolSize rtPoolSize{};
	rtPoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	rtPoolSize.descriptorCount = device->renderInfo.swapchainImageCount;

	std::vector<VkDescriptorPoolSize> poolSizes = {tlasPoolSize, imagePoolSize, dataPoolSize, rtPoolSize};

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = poolSizes.size();
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = device->renderInfo.swapchainImageCount;

	if (vkCreateDescriptorPool(device->getDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
		throw InitException("vkCreateDescriptorPool", "failed to create descriptor pool!");
	}
}

void RayTracingPipelinePoints::createBuffers() {
	globalDataBuffers.reserve(device->renderInfo.swapchainImageCount);
	countBuffers.reserve(device->renderInfo.swapchainImageCount);
	rtDataBuffers.reserve(device->renderInfo.swapchainImageCount);

	for (size_t i = 0; i < device->renderInfo.swapchainImageCount; ++i) {
		globalDataBuffers.emplace_back(device);

		globalDataBuffers.at(i).bufferSize = sizeof(RayTracingPipelinePoints::GlobalData);
		globalDataBuffers.at(i).usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		globalDataBuffers.at(i).properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		globalDataBuffers.at(i).init();

		countBuffers.emplace_back(device);

		countBuffers.at(i).bufferSize = sizeof(uint32_t);
		countBuffers.at(i).usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		countBuffers.at(i).properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		countBuffers.at(i).init();

		rtDataBuffers.emplace_back(device);

		rtDataBuffers.at(i).bufferSize = GraphicsObject::getRTDataSize() * rtDataPtrs.size();
		rtDataBuffers.at(i).usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		rtDataBuffers.at(i).properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		rtDataBuffers.at(i).init();
	}
}

void RayTracingPipelinePoints::createDescriptorSets() {
	descriptorSets.resize(device->renderInfo.swapchainImageCount);

	std::vector<VkDescriptorSetLayout> setLayouts;
	for (size_t i = 0; i < device->renderInfo.swapchainImageCount; ++i) {
		setLayouts.push_back(device->renderInfo.globalDescriptorSetLayout);
	}

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = setLayouts.size();
	allocInfo.pSetLayouts = setLayouts.data();

	if (vkAllocateDescriptorSets(device->getDevice(), &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
		throw InitException("vkAllocateDescriptorSets", "failed to allocate rt pipeline descriptor sets!");
	}

	VkWriteDescriptorSetAccelerationStructureKHR tlasWriteSetStructure{};
	tlasWriteSetStructure.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
	tlasWriteSetStructure.accelerationStructureCount = 1;
	tlasWriteSetStructure.pAccelerationStructures = &tlas.getAccelerationStructure();

	std::vector<VkWriteDescriptorSet> writeSets;

	for (size_t i = 0; i < device->renderInfo.swapchainImageCount; ++i) {
		VkWriteDescriptorSet tlasWriteSet{};
		tlasWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		tlasWriteSet.pNext = &tlasWriteSetStructure;
		tlasWriteSet.dstSet = descriptorSets.at(i);
		tlasWriteSet.dstBinding = 0;
		tlasWriteSet.descriptorCount = 1;
		tlasWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;

		writeSets.push_back(tlasWriteSet);

		VkDescriptorBufferInfo* countBufferInfo = new VkDescriptorBufferInfo();
		countBufferInfo->offset = 0;
		countBufferInfo->buffer = countBuffers.at(i).getBuffer();
		countBufferInfo->range = countBuffers.at(i).bufferSize;

		VkWriteDescriptorSet countWriteSet{};
		countWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		countWriteSet.pNext = nullptr;
		countWriteSet.dstSet = descriptorSets.at(i);
		countWriteSet.dstBinding = 1;
		countWriteSet.descriptorCount = 1;
		countWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		countWriteSet.pBufferInfo = countBufferInfo;

		writeSets.push_back(countWriteSet);

		VkDescriptorBufferInfo* dataBufferInfo = new VkDescriptorBufferInfo();
		dataBufferInfo->offset = 0;
		dataBufferInfo->buffer = globalDataBuffers.at(i).getBuffer();
		dataBufferInfo->range = globalDataBuffers.at(i).bufferSize;

		VkWriteDescriptorSet dataWriteSet{};
		dataWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		dataWriteSet.pNext = nullptr;
		dataWriteSet.dstSet = descriptorSets.at(i);
		dataWriteSet.dstBinding = 2;
		dataWriteSet.descriptorCount = 1;
		dataWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		dataWriteSet.pBufferInfo = dataBufferInfo;

		writeSets.push_back(dataWriteSet);

		VkDescriptorBufferInfo* rtBufferInfo = new VkDescriptorBufferInfo();
		rtBufferInfo->offset = 0;
		rtBufferInfo->buffer = rtDataBuffers.at(i).getBuffer();
		rtBufferInfo->range = rtDataBuffers.at(i).bufferSize;

		VkWriteDescriptorSet rtWriteSet{};
		rtWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		rtWriteSet.pNext = nullptr;
		rtWriteSet.dstSet = descriptorSets.at(i);
		rtWriteSet.dstBinding = 3;
		rtWriteSet.descriptorCount = 1;
		rtWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		rtWriteSet.pBufferInfo = rtBufferInfo;

		writeSets.push_back(rtWriteSet);
	}

	vkUpdateDescriptorSets(device->getDevice(), writeSets.size(), writeSets.data(), 0, VK_NULL_HANDLE);

	for (const VkWriteDescriptorSet& writeSet: writeSets) {
		if (writeSet.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) {
			delete writeSet.pImageInfo;
		} else if (writeSet.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
			delete writeSet.pBufferInfo;
		}
	}
}

void RayTracingPipelinePoints::createRayTracingPipeline() {
	std::vector<VkDescriptorSetLayout> setLayouts {
		device->renderInfo.globalDescriptorSetLayout,
	};

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = setLayouts.size();
	pipelineLayoutInfo.pSetLayouts = setLayouts.data();

	if (vkCreatePipelineLayout(device->getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw InitException("vkCreatePipelineLayout", "failed to create pipeline layout!");
	}

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

	VkShaderModule rgenShaderModule         = createShaderModule(device->getDevice(), rgenCode        );
	VkShaderModule rmissShaderModule        = createShaderModule(device->getDevice(), rmissCode       );
	VkShaderModule rshadowShaderModule      = createShaderModule(device->getDevice(), rshadowCode     );
	VkShaderModule rchitShaderModule        = createShaderModule(device->getDevice(), rchitCode       );

	// Ray generation group
	{
		VkPipelineShaderStageCreateInfo rgenShaderStageInfo{};
		rgenShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		rgenShaderStageInfo.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
		rgenShaderStageInfo.module = rgenShaderModule;
		rgenShaderStageInfo.pName = "main";
		shaderStages.push_back(rgenShaderStageInfo);

		VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
		shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		shaderGroup.generalShader = (uint32_t) shaderStages.size() - 1;
		shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
		shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
		shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
		shaderGroups.push_back(shaderGroup);
	}

	// Miss group
	{
		VkPipelineShaderStageCreateInfo rmissShaderStageInfo{};
		rmissShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		rmissShaderStageInfo.stage = VK_SHADER_STAGE_MISS_BIT_KHR;
		rmissShaderStageInfo.module = rmissShaderModule;
		rmissShaderStageInfo.pName = "main";
		shaderStages.push_back(rmissShaderStageInfo);

		VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
		shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		shaderGroup.generalShader = (uint32_t) shaderStages.size() - 1;
		shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
		shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
		shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
		shaderGroups.push_back(shaderGroup);
	}

	// Shadow group
	{
		VkPipelineShaderStageCreateInfo rshadowShaderStageInfo{};
		rshadowShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		rshadowShaderStageInfo.stage = VK_SHADER_STAGE_MISS_BIT_KHR;
		rshadowShaderStageInfo.module = rshadowShaderModule;
		rshadowShaderStageInfo.pName = "main";
		shaderStages.push_back(rshadowShaderStageInfo);

		VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
		shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		shaderGroup.generalShader = (uint32_t) shaderStages.size() - 1;
		shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
		shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
		shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
		shaderGroups.push_back(shaderGroup);
	}

	// Closest hit group
	{
		VkPipelineShaderStageCreateInfo rchitShaderStageInfo{};
		rchitShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		rchitShaderStageInfo.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		rchitShaderStageInfo.module = rchitShaderModule;
		rchitShaderStageInfo.pName = "main";
		shaderStages.push_back(rchitShaderStageInfo);

		VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
		shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
		shaderGroup.generalShader = VK_SHADER_UNUSED_KHR;
		shaderGroup.closestHitShader = (uint32_t) shaderStages.size() - 1;
		shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
		shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
		shaderGroups.push_back(shaderGroup);
	}

	/*
		Create the ray tracing pipeline
	*/
	VkRayTracingPipelineCreateInfoKHR rayTracingPipelineCI{};
	rayTracingPipelineCI.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
	rayTracingPipelineCI.stageCount = (uint32_t) shaderStages.size();
	rayTracingPipelineCI.pStages = shaderStages.data();
	rayTracingPipelineCI.groupCount = (uint32_t) shaderGroups.size();
	rayTracingPipelineCI.pGroups = shaderGroups.data();
	rayTracingPipelineCI.maxPipelineRayRecursionDepth = 2;
	rayTracingPipelineCI.layout = pipelineLayout;

	if (FuncLoad::vkCreateRayTracingPipelinesKHR(device->getDevice(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &rayTracingPipelineCI, nullptr, &pipeline) != VK_SUCCESS) {
		throw InitException("vkCreateRayTracingPipelinesKHR", "Could not create ray tracing pipeline!");
	}

	vkDestroyShaderModule(device->getDevice(), rgenShaderModule,    nullptr);
	vkDestroyShaderModule(device->getDevice(), rmissShaderModule,   nullptr);
	vkDestroyShaderModule(device->getDevice(), rshadowShaderModule, nullptr);
	vkDestroyShaderModule(device->getDevice(), rchitShaderModule,   nullptr);
}

void RayTracingPipelinePoints::createDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& uniformBindings, VkDescriptorSetLayout* pSetLayout) {
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = (uint32_t) uniformBindings.size();
	layoutInfo.pBindings = uniformBindings.data();

	if (vkCreateDescriptorSetLayout(device->getDevice(), &layoutInfo, nullptr, pSetLayout) != VK_SUCCESS) {
		throw InitException("vkCreateDescriptorSetLayout", "failed to create descriptor set layout!");
	}
}

std::vector<VkDescriptorSetLayoutBinding> RayTracingPipelinePoints::getUniformBindings() {
	VkDescriptorSetLayoutBinding accelerationStructureLayoutBinding{};
	accelerationStructureLayoutBinding.binding = 0;
	accelerationStructureLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
	accelerationStructureLayoutBinding.descriptorCount = 1;
	accelerationStructureLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

	VkDescriptorSetLayoutBinding countLayoutBinding{};
	countLayoutBinding.binding = 1;
	countLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	countLayoutBinding.descriptorCount = 1;
	countLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

	VkDescriptorSetLayoutBinding uniformBufferBinding{};
	uniformBufferBinding.binding = 2;
	uniformBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniformBufferBinding.descriptorCount = 1;
	uniformBufferBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR;

	VkDescriptorSetLayoutBinding objectBufferBinding{};
	objectBufferBinding.binding = 3;
	objectBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	objectBufferBinding.descriptorCount = 1;
	objectBufferBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

	return {accelerationStructureLayoutBinding, countLayoutBinding, uniformBufferBinding, objectBufferBinding};
}

uint32_t RayTracingPipelinePoints::getBindingSet() {
	return GLOBAL_BINDING_SET_INDEX;
}

const VkPipelineLayout& RayTracingPipelinePoints::getPipelineLayout() const {
	return pipelineLayout;
}

const VkPipeline& RayTracingPipelinePoints::getGraphicsPipeline() const {
	return pipeline;
}

uint32_t RayTracingPipelinePoints::alignedSize(uint32_t value, uint32_t alignment) {
	return (value + alignment - 1) & ~(alignment - 1);
}
