#include "shadow_tracer.h"


#define VISION_RGEN_SHADER "shadow_tracer_vision_raygen.spv"


ShadowTracer::ShadowTracer(Device* device)
:Renderer(device), device(device), descriptorCollection(device),
visionPipeline(device), renderSettingsBuffers(device) {}

ShadowTracer::~ShadowTracer() {}

void ShadowTracer::initRenderer() {
	createBuffers();
	createDescriptorCollection();
	createPipelineLayout();
	createVisionPipeline();
}

void ShadowTracer::cmdRenderFrame(size_t index, VkCommandBuffer commandBuffer) {
	descriptorCollection.cmdBind(index, commandBuffer, getPipelineLayout());

	visionPipeline.cmdExecutePipeline(commandBuffer);
}

void ShadowTracer::updateRendererUniforms(size_t index) {
	renderSettingsBuffers.at(index).passData((void*) &renderSettings);
}

void ShadowTracer::parseRendererInput(const InputEntry& inputEntry) {
	renderSettings.visionJumpCount = inputEntry.get<u_int32_t>("visionJumpCount");
	renderSettings.shadowTraceCount = inputEntry.get<u_int32_t>("shadowTraceCount");
}

void ShadowTracer::createBuffers() {
	renderSettingsBuffers.bufferProperties.bufferSize = sizeof(ShadowTracer::RenderSettings);
	renderSettingsBuffers.bufferProperties.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	renderSettingsBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	renderSettingsBuffers.init();
}

void ShadowTracer::createDescriptorCollection() {
	descriptorCollection.bufferDescriptors.resize(2);

	descriptorCollection.bufferDescriptors.at(0) = &renderSettingsBuffers;
	descriptorCollection.bufferDescriptors.at(1) = outputImages;

	descriptorCollection.init();

	descriptors.push_back(&descriptorCollection);
}

void ShadowTracer::createVisionPipeline() {
	visionPipeline.raygenShaders.push_back(VISION_RGEN_SHADER);
	visionPipeline.missShaders = Renderer::RMISS_SHADERS;
	visionPipeline.hitShaders = Renderer::RCHIT_SHADERS;

	visionPipeline.pipelineLayout = getPipelineLayout();

	visionPipeline.width = device->renderInfo.swapchainExtend.width;
	visionPipeline.height = device->renderInfo.swapchainExtend.height;

	visionPipeline.init();
}
