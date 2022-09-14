#include "median_denoiser.h"

#include "../ray_tracing_pipeline.h"

#define DENOISE_SHADER "denoise_median_comp.spv"

MedianDenoiser::MedianDenoiser(Device* device)
:Denoiser(device), settings(),
settingsBuffers(device), descriptorCollection(device), denoisePipeline(device) {}

MedianDenoiser::~MedianDenoiser() {}

void MedianDenoiser::initDenoiser() {
	createBuffers();
	createDescriptorCollection();
	createDenoisePipeline();
}

void MedianDenoiser::cmdRender(size_t index, VkCommandBuffer commandBuffer) {
	Denoiser::cmdPipelineBarrier(commandBuffer);
	descriptorCollection.cmdBind(index, commandBuffer);
	denoisePipeline.cmdExecutePipeline(commandBuffer);
}

void MedianDenoiser::updateUniforms(size_t /* index */) {
	// settingsBuffers.at(index).passData((void*) &settings);
}

void MedianDenoiser::parseInput(const InputEntry& /* inputEntry */) {}

void MedianDenoiser::createBuffers() {
	settingsBuffers.bufferProperties.bufferSize = sizeof(MedianDenoiser::Settings);
	settingsBuffers.bufferProperties.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	settingsBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	settingsBuffers.init();
}

void MedianDenoiser::createDescriptorCollection() {
	descriptorCollection.bufferDescriptors.resize(3);

	descriptorCollection.bufferDescriptors.at(0) = &inputImages;
	descriptorCollection.bufferDescriptors.at(1) = outputImages;
	descriptorCollection.bufferDescriptors.at(2) = &settingsBuffers;

	descriptorCollection.init();
}

void MedianDenoiser::createDenoisePipeline() {
	denoisePipeline.shaderPath = DENOISE_SHADER;
	denoisePipeline.pipelineLayout = descriptorCollection.getPipelineLayout();

	denoisePipeline.x = device->renderInfo.swapchainExtend.width;
	denoisePipeline.y = device->renderInfo.swapchainExtend.height;

	denoisePipeline.init();
}
