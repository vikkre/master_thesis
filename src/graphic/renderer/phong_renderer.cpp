#include "phong_renderer.h"


#define RCHIT_SHADER "phong_renderer_closesthit.spv"
#define RMISS_SHADER "phong_renderer_miss.spv"
#define RGEN_SHADER  "phong_renderer_raygen.spv"


PhongRenderer::PhongRenderer(Device* device)
:Renderer(device), device(device), descriptorCollection(device),
pipeline(device), objDataPtrs(),
tlas(device), objDataBuffers(device), globalDataBuffers(device), renderSettingsBuffers(device) {}

PhongRenderer::~PhongRenderer() {}

void PhongRenderer::initRenderer() {
	createTLAS();
	createBuffers();
	createDescriptorCollection();
	createPipeline();
}

void PhongRenderer::cmdRenderFrame(size_t index, VkCommandBuffer commandBuffer) {
	descriptorCollection.cmdBind(index, commandBuffer);

	pipeline.cmdExecutePipeline(commandBuffer);
}

void PhongRenderer::updateRendererUniforms(size_t index) {
	// globalDataBuffers.at(index).passData((void*) &globalData);
	renderSettingsBuffers.at(index).passData((void*) &renderSettings);

	for (size_t i = 0; i < objects.size(); ++i) {
		objects.at(i)->passBufferData(index);
		objDataBuffers.at(index).passData(objDataPtrs.at(i), i * GraphicsObject::getRTDataSize(), GraphicsObject::getRTDataSize());
	}
}

void PhongRenderer::parseRendererInput(const InputEntry& inputEntry) {
	renderSettings.backgroundColor = inputEntry.getVector<3, float>("backgroundColor");
	renderSettings.lightPosition = inputEntry.getVector<3, float>("lightPosition");
	renderSettings.diffuseConstant = inputEntry.get<float>("diffuseConstant");
	renderSettings.ambientConstant = inputEntry.get<float>("ambientConstant");
	renderSettings.specularConstant = inputEntry.get<float>("specularConstant");
	renderSettings.shininessConstant = inputEntry.get<float>("shininessConstant");
}

void PhongRenderer::createTLAS() {
	for (GraphicsObject* obj: objects) {
		GraphicsObject::ObjectInfo info = obj->getObjectInfo();
		tlas.bufferProperties.blasInstances.push_back(info.instance);
		objDataPtrs.push_back(info.dataPtr);
	}

	tlas.init();
}

void PhongRenderer::createBuffers() {
	objDataBuffers.bufferProperties.bufferSize = GraphicsObject::getRTDataSize() * objDataPtrs.size();
	objDataBuffers.bufferProperties.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	objDataBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	objDataBuffers.init();

	// globalDataBuffers.bufferProperties.bufferSize = sizeof(PhongRenderer::GlobalData);
	// globalDataBuffers.bufferProperties.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	// globalDataBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	// globalDataBuffers.init();

	renderSettingsBuffers.bufferProperties.bufferSize = sizeof(PhongRenderer::RenderSettings);
	renderSettingsBuffers.bufferProperties.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	renderSettingsBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	renderSettingsBuffers.init();
}

void PhongRenderer::createDescriptorCollection() {
	descriptorCollection.bufferDescriptors.resize(5);

	descriptorCollection.bufferDescriptors.at(0) = &tlas;
	descriptorCollection.bufferDescriptors.at(1) = &objDataBuffers;
	descriptorCollection.bufferDescriptors.at(2) = &globalDataBuffers;
	descriptorCollection.bufferDescriptors.at(3) = &renderSettingsBuffers;
	descriptorCollection.bufferDescriptors.at(4) = outputImages;

	descriptorCollection.init();
}

void PhongRenderer::createPipeline() {
	pipeline.raygenShaders.push_back(RGEN_SHADER);
	pipeline.missShaders.push_back(RMISS_SHADER);
	pipeline.hitShaders.push_back(RCHIT_SHADER);

	pipeline.pipelineLayout = descriptorCollection.getPipelineLayout();

	pipeline.width = device->renderInfo.swapchainExtend.width;
	pipeline.height = device->renderInfo.swapchainExtend.height;

	pipeline.init();
}
