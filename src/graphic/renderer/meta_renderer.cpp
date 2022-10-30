#include "meta_renderer.h"


#define RGEN_SHADER  "meta_renderer_raygen.spv"


MetaRenderer::MetaRenderer(Device* device)
:Renderer(device), device(device), descriptorCollection(device),
pipeline(device), renderSettingsBuffers(device) {}

MetaRenderer::~MetaRenderer() {}

void MetaRenderer::initRenderer() {
	createBuffers();
	createDescriptorCollection();
	createPipelineLayout();
	createPipeline();
}

void MetaRenderer::cmdRenderFrame(size_t index, VkCommandBuffer commandBuffer) {
	descriptorCollection.cmdBind(index, commandBuffer, getPipelineLayout());

	pipeline.cmdExecutePipeline(commandBuffer);
}

void MetaRenderer::updateRendererUniforms(size_t index) {
	renderSettingsBuffers.at(index).passData((void*) &renderSettings);
}

void MetaRenderer::parseRendererInput(const InputEntry& inputEntry) {
	renderSettings.resultType = inputEntry.get<uint32_t>("resultType");
	renderSettings.scaling = inputEntry.get<float>("scaling");
	renderSettings.lightJump = inputEntry.get<uint32_t>("lightJump");
}

void MetaRenderer::createBuffers() {
	renderSettingsBuffers.bufferProperties.bufferSize = sizeof(MetaRenderer::RenderSettings);
	renderSettingsBuffers.bufferProperties.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	renderSettingsBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	renderSettingsBuffers.init();
}

void MetaRenderer::createDescriptorCollection() {
	descriptorCollection.bufferDescriptors.resize(2);

	descriptorCollection.bufferDescriptors.at(0) = &renderSettingsBuffers;
	descriptorCollection.bufferDescriptors.at(1) = outputImages;

	descriptorCollection.init();

	descriptors.push_back(&descriptorCollection);
}

void MetaRenderer::createPipeline() {
	pipeline.raygenShaders.push_back(RGEN_SHADER);
	pipeline.missShaders = Renderer::RMISS_SHADERS;
	pipeline.hitShaders = Renderer::RCHIT_SHADERS;

	pipeline.pipelineLayout = getPipelineLayout();

	pipeline.width = device->renderInfo.swapchainExtend.width;
	pipeline.height = device->renderInfo.swapchainExtend.height;

	pipeline.init();
}
