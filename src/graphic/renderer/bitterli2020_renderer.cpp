#include "bitterli2020_renderer.h"


#define VISION_RGEN_SHADER "bitterli2020_vision_raygen.spv"


Bitterli2020::Bitterli2020(Device* device)
:Renderer(device), device(device), descriptorCollection(device),
visionPipeline(device), renderSettingsBuffers(device) {}

Bitterli2020::~Bitterli2020() {}

void Bitterli2020::initRenderer() {
	createBuffers();
	createDescriptorCollection();
	createPipelineLayout();
	createVisionPipeline();
}

void Bitterli2020::cmdRenderFrame(size_t index, VkCommandBuffer commandBuffer) {
	descriptorCollection.cmdBind(index, commandBuffer, getPipelineLayout());

	visionPipeline.cmdExecutePipeline(commandBuffer);
}

void Bitterli2020::updateRendererUniforms(size_t index) {
	renderSettingsBuffers.at(index).passData((void*) &renderSettings);
}

void Bitterli2020::parseRendererInput(const InputEntry& inputEntry) {
	renderSettings.visionJumpCount = inputEntry.get<u_int32_t>("visionJumpCount");
	renderSettings.shadowTraceCount = inputEntry.get<u_int32_t>("shadowTraceCount");
	renderSettings.candidateCount = inputEntry.get<u_int32_t>("candidateCount");
	renderSettings.usePrimitiveShadowTrace = inputEntry.get<u_int32_t>("usePrimitiveShadowTrace");
}

void Bitterli2020::createBuffers() {
	renderSettingsBuffers.bufferProperties.bufferSize = sizeof(Bitterli2020::RenderSettings);
	renderSettingsBuffers.bufferProperties.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	renderSettingsBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	renderSettingsBuffers.init();
}

void Bitterli2020::createDescriptorCollection() {
	descriptorCollection.bufferDescriptors.resize(2);

	descriptorCollection.bufferDescriptors.at(0) = &renderSettingsBuffers;
	descriptorCollection.bufferDescriptors.at(1) = outputImages;

	descriptorCollection.init();

	descriptors.push_back(&descriptorCollection);
}

void Bitterli2020::createVisionPipeline() {
	visionPipeline.raygenShaders.push_back(VISION_RGEN_SHADER);
	visionPipeline.missShaders = Renderer::RMISS_SHADERS;
	visionPipeline.hitShaders = Renderer::RCHIT_SHADERS;

	visionPipeline.pipelineLayout = getPipelineLayout();

	visionPipeline.width = device->renderInfo.swapchainExtend.width;
	visionPipeline.height = device->renderInfo.swapchainExtend.height;

	visionPipeline.init();
}
