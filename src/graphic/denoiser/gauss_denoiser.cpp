#include "gauss_denoiser.h"

#include "../ray_tracing_pipeline.h"

#define DENOISE_SHADER "denoise_gauss_comp.spv"

GaussDenoiser::GaussDenoiser(Device* device)
:Denoiser(device), settings(),
settingsBuffers(device), descriptorCollection(device), denoisePipeline(device) {}

GaussDenoiser::~GaussDenoiser() {}

void GaussDenoiser::initDenoiser() {
	createBuffers();
	createDescriptorCollection();
	createDenoisePipeline();
}

void GaussDenoiser::cmdRender(size_t index, VkCommandBuffer commandBuffer) {
	RayTracingPipeline::cmdRayTracingBarrier(commandBuffer);
	descriptorCollection.cmdBind(index, commandBuffer);
	denoisePipeline.cmdExecutePipeline(commandBuffer);
}

void GaussDenoiser::updateUniforms(size_t index) {
	settingsBuffers.at(index).passData((void*) &settings);
}

void GaussDenoiser::parseInput(const InputEntry& inputEntry) {
	settings.kernelSize = inputEntry.getInt("kernelSize");
	settings.sigma = inputEntry.getFloat("sigma");
}

void GaussDenoiser::createBuffers() {
	settingsBuffers.bufferProperties.bufferSize = sizeof(GaussDenoiser::Settings);
	settingsBuffers.bufferProperties.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	settingsBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	settingsBuffers.init();
}

void GaussDenoiser::createDescriptorCollection() {
	descriptorCollection.bufferDescriptors.resize(3);

	descriptorCollection.bufferDescriptors.at(0) = &inputImages;
	descriptorCollection.bufferDescriptors.at(1) = outputImages;
	descriptorCollection.bufferDescriptors.at(2) = &settingsBuffers;

	descriptorCollection.init();
}

void GaussDenoiser::createDenoisePipeline() {
	denoisePipeline.shaderPath = DENOISE_SHADER;
	denoisePipeline.pipelineLayout = descriptorCollection.getPipelineLayout();

	denoisePipeline.x = device->renderInfo.swapchainExtend.width;
	denoisePipeline.y = device->renderInfo.swapchainExtend.height;

	denoisePipeline.init();
}
