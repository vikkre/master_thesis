#include "bidirectional_path_tracer.h"


#define RGEN_SHADER "bidirectional_path_tracer_raygen.spv"


BidirectionalPathTracer::BidirectionalPathTracer(Device* device)
:Renderer(device), device(device), descriptorCollection(device),
visionPipeline(device), renderSettingsBuffers(device) {}

BidirectionalPathTracer::~BidirectionalPathTracer() {}

void BidirectionalPathTracer::initRenderer() {
	createBuffers();
	createDescriptorCollection();
	createPipelineLayout();
	createVisionPipeline();
}

void BidirectionalPathTracer::cmdRenderFrame(size_t index, VkCommandBuffer commandBuffer) {
	descriptorCollection.cmdBind(index, commandBuffer, getPipelineLayout());

	visionPipeline.cmdExecutePipeline(commandBuffer);
}

void BidirectionalPathTracer::updateRendererUniforms(size_t index) {
	renderSettingsBuffers.at(index).passData((void*) &renderSettings);
}

void BidirectionalPathTracer::parseRendererInput(const InputEntry& inputEntry) {
	renderSettings.visionJumpCount = inputEntry.get<u_int32_t>("visionJumpCount");
	renderSettings.lightJumpCount = inputEntry.get<u_int32_t>("lightJumpCount");
	renderSettings.maxDepth = inputEntry.get<u_int32_t>("maxDepth");
	renderSettings.raysPerPixelCount = inputEntry.get<u_int32_t>("raysPerPixelCount");
}

void BidirectionalPathTracer::createBuffers() {
	renderSettingsBuffers.bufferProperties.bufferSize = sizeof(BidirectionalPathTracer::RenderSettings);
	renderSettingsBuffers.bufferProperties.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	renderSettingsBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	renderSettingsBuffers.init();
}

void BidirectionalPathTracer::createDescriptorCollection() {
	descriptorCollection.addBuffer(0, &renderSettingsBuffers);
	descriptorCollection.addBuffer(1, outputImages);

	descriptorCollection.init();

	descriptors.push_back(&descriptorCollection);
}

void BidirectionalPathTracer::createVisionPipeline() {
	visionPipeline.raygenShaders.push_back(RGEN_SHADER);
	visionPipeline.missShaders = Renderer::RMISS_SHADERS;
	visionPipeline.hitShaders = Renderer::RCHIT_SHADERS;

	visionPipeline.pipelineLayout = getPipelineLayout();

	visionPipeline.width = device->renderInfo.swapchainExtend.width;
	visionPipeline.height = device->renderInfo.swapchainExtend.height;

	visionPipeline.init();
}
