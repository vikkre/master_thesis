#include "praktikums_renderer.h"


#define RGEN_SHADER "prakt_raygen.spv"
#define RCHIT_SHADER "prakt_closesthit.spv"
#define RMISS_SHADER "prakt_miss.spv"
#define RSHADOW_SHADER "prakt_shadow.spv"


PraktikumsRenderer::PraktikumsRenderer(Device* device)
:globalData(),
device(device), descriptorCollection(device),
pipeline(device), objects(), objDataPtrs(),
tlas(device),
globalDataBuffers(device), objDataBuffers(device) {}

PraktikumsRenderer::~PraktikumsRenderer() {}

void PraktikumsRenderer::init() {
	createTLAS();
	createBuffers();
	createDescriptorCollection();
	createPipeline();
}

void PraktikumsRenderer::cmdRender(size_t index, VkCommandBuffer commandBuffer) {
	descriptorCollection.cmdBind(index, commandBuffer);

	pipeline.cmdExecutePipeline(commandBuffer);
}

void PraktikumsRenderer::updateUniforms(size_t index) {
	globalData.viewInverse = Renderer::globalData.viewInverse;
	globalData.projInverse = Renderer::globalData.projInverse;
	globalData.view = Renderer::globalData.view;
	globalData.proj = Renderer::globalData.proj;
	globalData.backgroundColor = device->renderInfo.backgroundColor;
	globalData.lightPosition = device->renderInfo.lightPosition;

	globalDataBuffers.at(index).passData((void*) &globalData);

	for (size_t i = 0; i < objects.size(); ++i) {
		objects.at(i)->passBufferData(index);
		objDataBuffers.at(index).passData(objDataPtrs.at(i), i * GraphicsObject::getRTDataSize(), GraphicsObject::getRTDataSize());
	}
}

void PraktikumsRenderer::passObjects(const std::vector<GraphicsObject*>& objects) {
	this->objects = objects;
}

void PraktikumsRenderer::parseInput(const InputEntry& /* inputEntry */) {}

void PraktikumsRenderer::createTLAS() {
	for (GraphicsObject* obj: objects) {
		GraphicsObject::ObjectInfo info = obj->getObjectInfo();
		tlas.bufferProperties.blasInstances.push_back(info.instance);
		objDataPtrs.push_back(info.dataPtr);
	}

	tlas.init();
}

void PraktikumsRenderer::createBuffers() {
	globalDataBuffers.bufferProperties.bufferSize = sizeof(PraktikumsRenderer::GlobalData);
	globalDataBuffers.bufferProperties.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	globalDataBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	globalDataBuffers.init();

	objDataBuffers.bufferProperties.bufferSize = GraphicsObject::getRTDataSize() * objDataPtrs.size();
	objDataBuffers.bufferProperties.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	objDataBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	objDataBuffers.init();
}

void PraktikumsRenderer::createDescriptorCollection() {
	descriptorCollection.bufferDescriptors.resize(4);

	descriptorCollection.bufferDescriptors.at(0) = &tlas;
	descriptorCollection.bufferDescriptors.at(1) = outputImages;
	descriptorCollection.bufferDescriptors.at(2) = &globalDataBuffers;
	descriptorCollection.bufferDescriptors.at(3) = &objDataBuffers;

	descriptorCollection.init();
}

void PraktikumsRenderer::createPipeline() {
	pipeline.raygenShaders.push_back(RGEN_SHADER);
	pipeline.missShaders.push_back(RMISS_SHADER);
	pipeline.hitShaders.push_back(RCHIT_SHADER);
	pipeline.missShaders.push_back(RSHADOW_SHADER);

	pipeline.pipelineLayout = descriptorCollection.getPipelineLayout();

	pipeline.width = device->renderInfo.swapchainExtend.width;
	pipeline.height = device->renderInfo.swapchainExtend.height;

	pipeline.init();
}
