#include "unidirectional_path_tracer.h"


#define VISION_RGEN_SHADER "unidirectional_path_tracer_vision_raygen.spv"


UnidirectionalPathTracer::UnidirectionalPathTracer(Device* device)
:Renderer(device), device(device), descriptorCollection(device),
visionPipeline(device), renderSettingsBuffers(device) {}

UnidirectionalPathTracer::~UnidirectionalPathTracer() {}

void UnidirectionalPathTracer::initRenderer() {
	createBuffers();
	createDescriptorCollection();
	createPipelineLayout();
	createVisionPipeline();
}

void UnidirectionalPathTracer::cmdRenderFrame(size_t index, VkCommandBuffer commandBuffer) {
	descriptorCollection.cmdBind(index, commandBuffer, getPipelineLayout());

	visionPipeline.cmdExecutePipeline(commandBuffer);
}

void UnidirectionalPathTracer::updateRendererUniforms(size_t index) {
	renderSettingsBuffers.at(index).passData((void*) &renderSettings);
}

void UnidirectionalPathTracer::parseRendererInput(const InputEntry& inputEntry) {
	renderSettings.visionJumpCount = inputEntry.get<u_int32_t>("visionJumpCount");
	renderSettings.visionRayPerPixelCount = inputEntry.get<u_int32_t>("visionRayPerPixelCount");
}

void UnidirectionalPathTracer::createBuffers() {
	renderSettingsBuffers.bufferProperties.bufferSize = sizeof(UnidirectionalPathTracer::RenderSettings);
	renderSettingsBuffers.bufferProperties.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	renderSettingsBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	renderSettingsBuffers.init();
}

void UnidirectionalPathTracer::createDescriptorCollection() {
	descriptorCollection.bufferDescriptors.resize(2);

	descriptorCollection.bufferDescriptors.at(0) = &renderSettingsBuffers;
	descriptorCollection.bufferDescriptors.at(1) = outputImages;

	descriptorCollection.init();

	descriptors.push_back(&descriptorCollection);
}

void UnidirectionalPathTracer::createVisionPipeline() {
	visionPipeline.raygenShaders.push_back(VISION_RGEN_SHADER);
	visionPipeline.missShaders = Renderer::RMISS_SHADERS;
	visionPipeline.hitShaders = Renderer::RCHIT_SHADERS;

	visionPipeline.pipelineLayout = getPipelineLayout();

	visionPipeline.width = device->renderInfo.swapchainExtend.width;
	visionPipeline.height = device->renderInfo.swapchainExtend.height;

	visionPipeline.init();
}
