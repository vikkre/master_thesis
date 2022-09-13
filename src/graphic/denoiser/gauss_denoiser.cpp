#include "gauss_denoiser.h"

#include "../ray_tracing_pipeline.h"


#define DENOISE_SHADER "denoise_gauss_comp.spv"

#define GAUSS_ARRAY_MAX_SIZE 20


GaussDenoiser::GaussDenoiser(Device* device)
:Denoiser(device), settings(),
settingsBuffers(device), gaussBuffers(device),
descriptorCollection(device), denoisePipeline(device) {}

GaussDenoiser::~GaussDenoiser() {}

void GaussDenoiser::initDenoiser() {
	createBuffers();
	createDescriptorCollection();
	createDenoisePipeline();
}

void GaussDenoiser::cmdRender(size_t index, VkCommandBuffer commandBuffer) {
	Denoiser::cmdPipelineBarrier(commandBuffer);
	descriptorCollection.cmdBind(index, commandBuffer);
	denoisePipeline.cmdExecutePipeline(commandBuffer);
}

void GaussDenoiser::updateUniforms(size_t index) {
	float gauss[GAUSS_ARRAY_MAX_SIZE];
	float sum = 0.0f;
	for (unsigned int i = 0; i < settings.kernelSize; i++) {
		float a = (float(i) - (float(settings.kernelSize) - 1.0f));
		a = a*a;
		float b = (2.0f * settings.sigma * settings.sigma);
		float r = exp(-a/b);
		gauss[i] = r;
		sum += r;
	}
	sum = 1.0f / sum;
	for (unsigned int i = 0; i < settings.kernelSize; i++) {
		gauss[i] *= sum;
	}

	settingsBuffers.at(index).passData((void*) &settings);
	gaussBuffers.at(index).passData((void*) gauss);
}

void GaussDenoiser::parseInput(const InputEntry& inputEntry) {
	settings.kernelSize = inputEntry.get<u_int32_t>("kernelSize");
	settings.sigma = inputEntry.get<float>("sigma");
}

void GaussDenoiser::createBuffers() {
	settingsBuffers.bufferProperties.bufferSize = sizeof(GaussDenoiser::Settings);
	settingsBuffers.bufferProperties.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	settingsBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	settingsBuffers.init();

	gaussBuffers.bufferProperties.bufferSize = sizeof(float) * GAUSS_ARRAY_MAX_SIZE;
	gaussBuffers.bufferProperties.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	gaussBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	gaussBuffers.init();
}

void GaussDenoiser::createDescriptorCollection() {
	descriptorCollection.bufferDescriptors.resize(4);

	descriptorCollection.bufferDescriptors.at(0) = &inputImages;
	descriptorCollection.bufferDescriptors.at(1) = outputImages;
	descriptorCollection.bufferDescriptors.at(2) = &settingsBuffers;
	descriptorCollection.bufferDescriptors.at(3) = &gaussBuffers;

	descriptorCollection.init();
}

void GaussDenoiser::createDenoisePipeline() {
	denoisePipeline.shaderPath = DENOISE_SHADER;
	denoisePipeline.pipelineLayout = descriptorCollection.getPipelineLayout();

	denoisePipeline.x = device->renderInfo.swapchainExtend.width;
	denoisePipeline.y = device->renderInfo.swapchainExtend.height;

	denoisePipeline.init();
}