#include "m17_denoiser.h"

#include "../ray_tracing_pipeline.h"


#define DENOISE_TEMPORAL_SHADER       "denoise_m17_temporal_comp.spv"
#define DENOISE_PRE_BILATERAL_SHADER  "denoise_m17_pre_bilateral_comp.spv"
#define DENOISE_FIREFLY_SHADER        "denoise_m17_firefly_comp.spv"
#define DENOISE_DISOCCLUSION_SHADER   "denoise_m17_disocclusion_comp.spv"
#define DENOISE_POST_BILATERAL_SHADER "denoise_m17_post_bilateral_comp.spv"


M17Denoiser::M17Denoiser(Device* device)
:Denoiser(device), settings(),
descriptorCollection(device), denoiseTemporalPipeline(device), denoisePreBilateralPipeline(device),
denoiseFireflyPipeline(device), denoiseDisocclusionPipeline(device), denoisePostBilateralPipeline(device),
settingsBuffers(device), temporalImage(device), historicalBuffer(device),
fireflyFreeImage(device), confidenceBuffer(device), kernelBuffer(device) {}

M17Denoiser::~M17Denoiser() {}

void M17Denoiser::initDenoiser() {
	createBuffers();
	createDescriptorCollection();
	createPipelines();
}

void M17Denoiser::cmdRender(size_t index, VkCommandBuffer commandBuffer) {
	Denoiser::cmdPipelineBarrier(commandBuffer);
	descriptorCollection.cmdBind(index, commandBuffer);

	denoiseTemporalPipeline.cmdExecutePipeline(commandBuffer);
	Denoiser::cmdInnerBarrier(commandBuffer);
	denoisePreBilateralPipeline.cmdExecutePipeline(commandBuffer);
	denoiseFireflyPipeline.cmdExecutePipeline(commandBuffer);
	denoiseDisocclusionPipeline.cmdExecutePipeline(commandBuffer);
	Denoiser::cmdInnerBarrier(commandBuffer);
	denoisePostBilateralPipeline.cmdExecutePipeline(commandBuffer);
}

void M17Denoiser::updateUniforms(size_t index) {
	settingsBuffers.at(index).passData((void*) &settings);
}

void M17Denoiser::parseInput(const InputEntry& inputEntry) {
	settings.historicalBlendingFactor = inputEntry.get<float>("historicalBlendingFactor");
	settings.smoothstepEdge0 = inputEntry.get<float>("smoothstepEdge0");
	settings.smoothstepEdge1 = inputEntry.get<float>("smoothstepEdge1");
}

void M17Denoiser::createBuffers() {
	settingsBuffers.bufferProperties.bufferSize = sizeof(M17Denoiser::Settings);
	settingsBuffers.bufferProperties.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	settingsBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	settingsBuffers.init();

	ImageBuffer::Properties intermediateImageProperties = outputImages->bufferProperties;

	std::vector<MultiBufferDescriptor<ImageBuffer>*> imageBuffers = {
		&temporalImage,
		&historicalBuffer,
		&fireflyFreeImage,
		&confidenceBuffer,
		&kernelBuffer
	};

	for (MultiBufferDescriptor<ImageBuffer>* imageBuffer: imageBuffers) {
		imageBuffer->bufferProperties = intermediateImageProperties;
		imageBuffer->init();
	}
}

void M17Denoiser::createDescriptorCollection() {
	descriptorCollection.bufferDescriptors.resize(8);

	descriptorCollection.bufferDescriptors.at(0) = &inputImages;
	descriptorCollection.bufferDescriptors.at(1) = outputImages;
	descriptorCollection.bufferDescriptors.at(2) = &settingsBuffers;
	descriptorCollection.bufferDescriptors.at(3) = &temporalImage;
	descriptorCollection.bufferDescriptors.at(4) = &historicalBuffer;
	descriptorCollection.bufferDescriptors.at(5) = &fireflyFreeImage;
	descriptorCollection.bufferDescriptors.at(6) = &confidenceBuffer;
	descriptorCollection.bufferDescriptors.at(7) = &kernelBuffer;

	descriptorCollection.init();
}

void M17Denoiser::createPipelines() {
	denoiseTemporalPipeline.shaderPath = DENOISE_TEMPORAL_SHADER;
	denoisePreBilateralPipeline.shaderPath = DENOISE_PRE_BILATERAL_SHADER;
	denoiseFireflyPipeline.shaderPath = DENOISE_FIREFLY_SHADER;
	denoiseDisocclusionPipeline.shaderPath = DENOISE_DISOCCLUSION_SHADER;
	denoisePostBilateralPipeline.shaderPath = DENOISE_POST_BILATERAL_SHADER;

	std::vector<ComputePipeline*> pipelines = {
		&denoiseTemporalPipeline,
		&denoisePreBilateralPipeline,
		&denoiseFireflyPipeline,
		&denoiseDisocclusionPipeline,
		&denoisePostBilateralPipeline,
	};

	for (ComputePipeline* pipeline: pipelines) {
		pipeline->pipelineLayout = descriptorCollection.getPipelineLayout();

		pipeline->x = device->renderInfo.swapchainExtend.width;
		pipeline->y = device->renderInfo.swapchainExtend.height;

		pipeline->init();
	}
}
