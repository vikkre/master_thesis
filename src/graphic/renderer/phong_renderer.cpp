#include "phong_renderer.h"


#define RGEN_SHADER  "phong_renderer_raygen.spv"


PhongRenderer::PhongRenderer(Device* device)
:Renderer(device), device(device), descriptorCollection(device),
pipeline(device), renderSettingsBuffers(device) {}

PhongRenderer::~PhongRenderer() {}

void PhongRenderer::initRenderer() {
	createBuffers();
	createDescriptorCollection();
	createPipelineLayout();
	createPipeline();
}

void PhongRenderer::cmdRenderFrame(size_t index, VkCommandBuffer commandBuffer) {
	descriptorCollection.cmdBind(index, commandBuffer, getPipelineLayout());

	pipeline.cmdExecutePipeline(commandBuffer);
}

void PhongRenderer::updateRendererUniforms(size_t index) {
	renderSettingsBuffers.at(index).passData((void*) &renderSettings);
}

void PhongRenderer::parseRendererInput(const InputEntry& inputEntry) {
	renderSettings.diffuseConstant = inputEntry.get<float>("diffuseConstant");
	renderSettings.ambientConstant = inputEntry.get<float>("ambientConstant");
	renderSettings.specularConstant = inputEntry.get<float>("specularConstant");
	renderSettings.shininessConstant = inputEntry.get<float>("shininessConstant");
}

void PhongRenderer::createBuffers() {
	renderSettingsBuffers.bufferProperties.bufferSize = sizeof(PhongRenderer::RenderSettings);
	renderSettingsBuffers.bufferProperties.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	renderSettingsBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	renderSettingsBuffers.init();
}

void PhongRenderer::createDescriptorCollection() {
	descriptorCollection.bufferDescriptors.resize(2);

	descriptorCollection.bufferDescriptors.at(0) = &renderSettingsBuffers;
	descriptorCollection.bufferDescriptors.at(1) = outputImages;

	descriptorCollection.init();

	descriptors.push_back(&descriptorCollection);
}

void PhongRenderer::createPipeline() {
	pipeline.raygenShaders.push_back(RGEN_SHADER);
	pipeline.missShaders = Renderer::RMISS_SHADERS;
	pipeline.hitShaders = Renderer::RCHIT_SHADERS;

	pipeline.pipelineLayout = getPipelineLayout();

	pipeline.width = device->renderInfo.swapchainExtend.width;
	pipeline.height = device->renderInfo.swapchainExtend.height;

	pipeline.init();
}
