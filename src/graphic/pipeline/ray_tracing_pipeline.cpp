#include "ray_tracing_pipeline.h"

#include "../helper/function_load.h"
#include "../device.h"
#include "../graphics_object.h"


#define GLOBAL_BINDING_SET_INDEX 0

#define RGEN_SHADER "raygen.spv"
#define RCHIT_SHADER "closesthit.spv"
#define RMISS_SHADER "miss.spv"
#define RSHADOW_SHADER "shadow.spv"


RayTracingPipeline::RayTracingPipeline(Device* device)
:Pipeline(device),
raygenShaders(), missShaders(), hitShaders(),
bufferDescriptors(),
width(1), height(1),
rayTracingPipelineProperties(),
raygenShaderBindingTable(device), missShaderBindingTable(device), hitShaderBindingTable(device),
pipeline(VK_NULL_HANDLE), pipelineLayout(VK_NULL_HANDLE),
descriptorPool(VK_NULL_HANDLE), descriptorSets(), descriptorSetLayout(VK_NULL_HANDLE) {}

RayTracingPipeline::~RayTracingPipeline() {
	vkDestroyDescriptorPool(device->getDevice(), descriptorPool, nullptr);
	vkDestroyPipeline(device->getDevice(), pipeline, nullptr);
	vkDestroyPipelineLayout(device->getDevice(), pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(device->getDevice(), descriptorSetLayout, nullptr);
	for (BufferDescriptor* bd: bufferDescriptors) delete bd;
}

void RayTracingPipeline::init() {
	getProperties();

	createDescriptorSetLayout();
	createDescriptorPool();
	createDescriptorSets();
	createRayTracingPipeline();
	createShaderBindingTable();
}

void RayTracingPipeline::getProperties() {
	rayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
	VkPhysicalDeviceProperties2 deviceProperties2{};
	deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	deviceProperties2.pNext = &rayTracingPipelineProperties;
	vkGetPhysicalDeviceProperties2(device->getPhysicalDevice(), &deviceProperties2);
}

void RayTracingPipeline::cmdExecutePipeline(size_t index, const VkCommandBuffer* commandBuffer) {
	vkCmdBindDescriptorSets(
		*commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
		pipelineLayout,
		GLOBAL_BINDING_SET_INDEX, 1, &descriptorSets.at(index),
		0, nullptr
	);

	vkCmdBindPipeline(*commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline);

	const uint32_t handleSizeAligned = alignedSize(rayTracingPipelineProperties.shaderGroupHandleSize, rayTracingPipelineProperties.shaderGroupHandleAlignment);

	VkStridedDeviceAddressRegionKHR generalShaderSbtEntry{};
	generalShaderSbtEntry.stride = handleSizeAligned;
	generalShaderSbtEntry.size = handleSizeAligned;

	VkStridedDeviceAddressRegionKHR raygenShaderSbtEntry = generalShaderSbtEntry;
	raygenShaderSbtEntry.deviceAddress = raygenShaderBindingTable.getAddress();

	VkStridedDeviceAddressRegionKHR missShaderSbtEntry = generalShaderSbtEntry;
	missShaderSbtEntry.deviceAddress = missShaderBindingTable.getAddress();

	VkStridedDeviceAddressRegionKHR hitShaderSbtEntry = generalShaderSbtEntry;
	hitShaderSbtEntry.deviceAddress = hitShaderBindingTable.getAddress();

	VkStridedDeviceAddressRegionKHR callableShaderSbtEntry{};

	FuncLoad::vkCmdTraceRaysKHR(
		*commandBuffer,
		&raygenShaderSbtEntry,
		&missShaderSbtEntry,
		&hitShaderSbtEntry,
		&callableShaderSbtEntry,
		width, height, 1
	);
}

void RayTracingPipeline::createShaderBindingTable() {
	const uint32_t handleSize = rayTracingPipelineProperties.shaderGroupHandleSize;
	const uint32_t handleSizeAligned = alignedSize(rayTracingPipelineProperties.shaderGroupHandleSize, rayTracingPipelineProperties.shaderGroupHandleAlignment);
	const uint32_t groupCount = raygenShaders.size() + missShaders.size() + hitShaders.size();
	const uint32_t sbtSize = groupCount * handleSizeAligned;

	std::vector<uint8_t> shaderHandleStorage(sbtSize);
	if(FuncLoad::vkGetRayTracingShaderGroupHandlesKHR(device->getDevice(), pipeline, 0, groupCount, sbtSize, shaderHandleStorage.data()) != VK_SUCCESS) {
		throw InitException("vkGetRayTracingShaderGroupHandlesKHR", "could not get shader group handles");
	};

	const VkBufferUsageFlags bufferUsageFlags = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	const VkMemoryPropertyFlags memoryUsageFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	
	raygenShaderBindingTable.bufferSize = handleSize * raygenShaders.size();
	raygenShaderBindingTable.usage      = bufferUsageFlags;
	raygenShaderBindingTable.properties = memoryUsageFlags;
	raygenShaderBindingTable.init();
	
	missShaderBindingTable.bufferSize = handleSize * missShaders.size();
	missShaderBindingTable.usage      = bufferUsageFlags;
	missShaderBindingTable.properties = memoryUsageFlags;
	missShaderBindingTable.init();
	
	hitShaderBindingTable.bufferSize = handleSize * hitShaders.size();
	hitShaderBindingTable.usage      = bufferUsageFlags;
	hitShaderBindingTable.properties = memoryUsageFlags;
	hitShaderBindingTable.init();

	unsigned int raygenTableOffset = 0;
	unsigned int missTableOffset = raygenShaders.size();
	unsigned int hitTableOffset = raygenShaders.size() + missShaders.size();

	raygenShaderBindingTable.passData((void*) &shaderHandleStorage[handleSizeAligned * raygenTableOffset]);
	missShaderBindingTable.passData  ((void*) &shaderHandleStorage[handleSizeAligned * missTableOffset]);
	hitShaderBindingTable.passData   ((void*) &shaderHandleStorage[handleSizeAligned * hitTableOffset]);
}

void RayTracingPipeline::createDescriptorPool() {
	std::vector<VkDescriptorPoolSize> poolSizes;

	for (BufferDescriptor* bufferDescriptor: bufferDescriptors) {
		VkDescriptorPoolSize poolSize{};
		poolSize.type = bufferDescriptor->getBuffer()->getDescriptorType();
		poolSize.descriptorCount = device->renderInfo.swapchainImageCount;
		poolSizes.push_back(poolSize);
	}

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = poolSizes.size();
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = device->renderInfo.swapchainImageCount;

	if (vkCreateDescriptorPool(device->getDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
		throw InitException("vkCreateDescriptorPool", "failed to create descriptor pool!");
	}
}

void RayTracingPipeline::createDescriptorSets() {
	descriptorSets.resize(device->renderInfo.swapchainImageCount);

	std::vector<VkDescriptorSetLayout> setLayouts;
	for (size_t i = 0; i < device->renderInfo.swapchainImageCount; ++i) {
		setLayouts.push_back(descriptorSetLayout);
	}

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = setLayouts.size();
	allocInfo.pSetLayouts = setLayouts.data();

	if (vkAllocateDescriptorSets(device->getDevice(), &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
		throw InitException("vkAllocateDescriptorSets", "failed to allocate rt pipeline descriptor sets!");
	}

	std::vector<VkWriteDescriptorSet> writeSets;

	for (BufferDescriptor* bufferDescriptor: bufferDescriptors) {
		bufferDescriptor->getWriteDescriptorSets(writeSets, descriptorSets);
	}

	vkUpdateDescriptorSets(device->getDevice(), writeSets.size(), writeSets.data(), 0, VK_NULL_HANDLE);
}

void RayTracingPipeline::createRayTracingPipeline() {
	std::vector<VkDescriptorSetLayout> setLayouts {
		descriptorSetLayout,
	};

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = setLayouts.size();
	pipelineLayoutInfo.pSetLayouts = setLayouts.data();

	if (vkCreatePipelineLayout(device->getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw InitException("vkCreatePipelineLayout", "failed to create pipeline layout!");
	}

	std::vector<VkShaderModule> shaderModules;
	std::vector<VkPipelineShaderStageCreateInfo> shaderStageInfos;
	std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups;


	struct ShaderStage {
		std::vector<char> shaderCode;
		VkShaderStageFlagBits shaderStageFlags;
	};
	std::vector<ShaderStage> shaderStages;
	for (const std::string& shaderCode: raygenShaders) shaderStages.push_back({readFile(shaderCode), VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	for (const std::string& shaderCode: missShaders)   shaderStages.push_back({readFile(shaderCode), VK_SHADER_STAGE_MISS_BIT_KHR});
	for (const std::string& shaderCode: hitShaders)    shaderStages.push_back({readFile(shaderCode), VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR});

	for (const ShaderStage& shaderStage: shaderStages) {
		VkShaderModule shaderModule = createShaderModule(device->getDevice(), shaderStage.shaderCode);
		shaderModules.push_back(shaderModule);

		VkPipelineShaderStageCreateInfo shaderStageInfo{};
		shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageInfo.stage = shaderStage.shaderStageFlags;
		shaderStageInfo.module = shaderModule;
		shaderStageInfo.pName = "main";
		shaderStageInfos.push_back(shaderStageInfo);

		VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
		shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
		shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;

		if (shaderStage.shaderStageFlags == VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR) {
			shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
			shaderGroup.generalShader = VK_SHADER_UNUSED_KHR;
			shaderGroup.closestHitShader = (uint32_t) shaderStageInfos.size() - 1;
		} else {
			shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
			shaderGroup.generalShader = (uint32_t) shaderStageInfos.size() - 1;
			shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
		}

		shaderGroups.push_back(shaderGroup);
	}

	VkRayTracingPipelineCreateInfoKHR rayTracingPipelineCI{};
	rayTracingPipelineCI.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
	rayTracingPipelineCI.stageCount = (uint32_t) shaderStageInfos.size();
	rayTracingPipelineCI.pStages = shaderStageInfos.data();
	rayTracingPipelineCI.groupCount = (uint32_t) shaderGroups.size();
	rayTracingPipelineCI.pGroups = shaderGroups.data();
	rayTracingPipelineCI.maxPipelineRayRecursionDepth = 2;
	rayTracingPipelineCI.layout = pipelineLayout;

	if (FuncLoad::vkCreateRayTracingPipelinesKHR(device->getDevice(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &rayTracingPipelineCI, nullptr, &pipeline) != VK_SUCCESS) {
		throw InitException("vkCreateRayTracingPipelinesKHR", "Could not create ray tracing pipeline!");
	}

	for (VkShaderModule& shaderModule: shaderModules) {
		vkDestroyShaderModule(device->getDevice(), shaderModule, nullptr);
	}
}

void RayTracingPipeline::createDescriptorSetLayout() {
	std::vector<VkDescriptorSetLayoutBinding> layoutBindings;

	for (BufferDescriptor* bufferDescriptor: bufferDescriptors) {
		layoutBindings.push_back(bufferDescriptor->getLayoutBinding());
	}
	
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = (uint32_t) layoutBindings.size();
	layoutInfo.pBindings = layoutBindings.data();

	if (vkCreateDescriptorSetLayout(device->getDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
		throw InitException("vkCreateDescriptorSetLayout", "failed to create descriptor set layout!");
	}
}

uint32_t RayTracingPipeline::getBindingSet() {
	return GLOBAL_BINDING_SET_INDEX;
}

const VkPipelineLayout& RayTracingPipeline::getPipelineLayout() const {
	return pipelineLayout;
}

const VkPipeline& RayTracingPipeline::getGraphicsPipeline() const {
	return pipeline;
}

uint32_t RayTracingPipeline::alignedSize(uint32_t size, uint32_t alignment) {
	return (size + alignment - 1) & ~(alignment - 1);
}
