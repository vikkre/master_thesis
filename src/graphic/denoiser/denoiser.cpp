#include "denoiser.h"


Denoiser::Denoiser(Device* device)
:device(device), inputImages(device), outputImages(nullptr) {}

Denoiser::~Denoiser() {}

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

void Denoiser::createInputImages() {
	inputImages.bufferProperties = outputImages->bufferProperties;
	inputImages.init();
}
