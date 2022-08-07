#include "compute_pipeline.h"


ComputePipeline::ComputePipeline(Device* device)
:shaderPath(), pipelineLayout(VK_NULL_HANDLE), x(1), y(1), z(1),
device(device), pipeline(VK_NULL_HANDLE) {}

ComputePipeline::~ComputePipeline() {
	vkDestroyPipeline(device->getDevice(), pipeline, nullptr);
}

void ComputePipeline::init() {
	createPipeline();
}

void ComputePipeline::cmdExecutePipeline(const VkCommandBuffer* commandBuffer) {
	vkCmdBindPipeline(*commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
	vkCmdDispatch(*commandBuffer, x, y, z);
}

void ComputePipeline::createPipeline() {
	std::ifstream file(shaderPath, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		std::string text = "failed to open file \"";
		text += shaderPath;
		text += "\"!";
		throw InitException("Pipeline readFile", text);
	}

	size_t fileSize = (size_t) file.tellg();
	std::vector<char> shaderCode(fileSize);

	file.seekg(0);
	file.read(shaderCode.data(), fileSize);

	file.close();

	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = shaderCode.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device->getDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw InitException("vkCreateShaderModule", "failed to create shader module!");
	}

	VkPipelineShaderStageCreateInfo shaderStageInfo{};
	shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	shaderStageInfo.module = shaderModule;
	shaderStageInfo.pName = "main";

	VkComputePipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	pipelineInfo.stage = shaderStageInfo;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = 0;

	if (vkCreateComputePipelines(device->getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
		throw InitException("vkCreateComputePipelines", "failed to create compute pipeline!");
	}

	vkDestroyShaderModule(device->getDevice(), shaderModule, nullptr);
}
