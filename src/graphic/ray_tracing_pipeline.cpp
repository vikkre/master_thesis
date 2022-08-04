#include "ray_tracing_pipeline.h"

#include "helper/function_load.h"
#include "device.h"


std::vector<char> readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		std::string text = "failed to open file \"";
		text += filename;
		text += "\"!";
		throw InitException("Pipeline readFile", text);
	}

	size_t fileSize = (size_t) file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& code) {
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw InitException("vkCreateShaderModule", "failed to create shader module!");
	}

	return shaderModule;
}


RayTracingPipeline::RayTracingPipeline(Device* device)
:raygenShaders(), missShaders(), hitShaders(),
width(1), height(1), depth(1), pipelineLayout(VK_NULL_HANDLE),
device(device), rayTracingPipelineProperties(),
raygenShaderBindingTable(device), missShaderBindingTable(device), hitShaderBindingTable(device),
pipeline(VK_NULL_HANDLE) {}

RayTracingPipeline::~RayTracingPipeline() {
	vkDestroyPipeline(device->getDevice(), pipeline, nullptr);
}

void RayTracingPipeline::init() {
	getProperties();

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

void RayTracingPipeline::cmdExecutePipeline(const VkCommandBuffer* commandBuffer) {
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
		width, height, depth
	);
}

void RayTracingPipeline::cmdRayTracingBarrier(const VkCommandBuffer* commandBuffer) {
	vkCmdPipelineBarrier(
		*commandBuffer,
		VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
		0,
		0, nullptr,
		0, nullptr,
		0, nullptr
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

	DataBuffer::Properties generalProperties;
	generalProperties.usage = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	generalProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	
	raygenShaderBindingTable.properties = generalProperties;
	raygenShaderBindingTable.properties.bufferSize = handleSize * raygenShaders.size();
	raygenShaderBindingTable.init();
	
	missShaderBindingTable.properties = generalProperties;
	missShaderBindingTable.properties.bufferSize = handleSize * missShaders.size();
	missShaderBindingTable.init();
	
	hitShaderBindingTable.properties = generalProperties;
	hitShaderBindingTable.properties.bufferSize = handleSize * hitShaders.size();
	hitShaderBindingTable.init();

	unsigned int raygenTableOffset = 0;
	unsigned int missTableOffset = raygenShaders.size();
	unsigned int hitTableOffset = raygenShaders.size() + missShaders.size();

	raygenShaderBindingTable.passData((void*) &shaderHandleStorage[handleSizeAligned * raygenTableOffset]);
	missShaderBindingTable.passData  ((void*) &shaderHandleStorage[handleSizeAligned * missTableOffset]);
	hitShaderBindingTable.passData   ((void*) &shaderHandleStorage[handleSizeAligned * hitTableOffset]);
}

void RayTracingPipeline::createRayTracingPipeline() {
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

uint32_t RayTracingPipeline::alignedSize(uint32_t size, uint32_t alignment) {
	return (size + alignment - 1) & ~(alignment - 1);
}
