#include "denoiser.h"


Denoiser::Denoiser(Device* device)
:device(device), inputImages(device), outputImages(nullptr) {}

Denoiser::~Denoiser() {
	vkDestroyPipelineLayout(device->getDevice(), pipelineLayout, nullptr);
}

void Denoiser::init() {
	createInputImages();
	initDenoiser();
}

void Denoiser::initDenoiser() {}

MultiBufferDescriptor<ImageBuffer>* Denoiser::getInputImageBuffer() {
	return &inputImages;
}

void Denoiser::setOutputImageBuffer(MultiBufferDescriptor<ImageBuffer>* outputImageBuffer) {
	outputImages = outputImageBuffer;
}

void Denoiser::cmdPipelineBarrier(VkCommandBuffer commandBuffer) {
	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		0, nullptr
	);
}

void Denoiser::cmdInnerBarrier(VkCommandBuffer commandBuffer) {
	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		0, nullptr
	);
}

void Denoiser::createInputImages() {
	inputImages.bufferProperties = outputImages->bufferProperties;
	inputImages.init();
}

void Denoiser::createPipelineLayout() {
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts(descriptors.size());

	for (const DescriptorCollection* descriptor: descriptors) {
		descriptorSetLayouts.at(descriptor->bindingSetIndex) = descriptor->getLayout();
	}

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size();
	pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();

	if (vkCreatePipelineLayout(device->getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw InitException("vkCreatePipelineLayout", "failed to create pipeline layout!");
	}
}

VkPipelineLayout Denoiser::getPipelineLayout() {
	return pipelineLayout;
}
