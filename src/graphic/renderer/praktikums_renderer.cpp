#include "praktikums_renderer.h"


#define RGEN_SHADER "prakt_raygen.spv"
#define RCHIT_SHADER "prakt_closesthit.spv"
#define RMISS_SHADER "prakt_miss.spv"
#define RSHADOW_SHADER "prakt_shadow.spv"


PraktikumsRenderer::PraktikumsRenderer(Device* device)
:objects(), globalData(),
device(device), descriptorCollection(device),
pipeline(device), rtDataPtrs(),
tlas(device), storageImages(device),
globalDataBuffers(device), rtDataBuffers(device) {}

PraktikumsRenderer::~PraktikumsRenderer() {}

void PraktikumsRenderer::init() {
	createTLAS();
	createBuffers();
	createDescriptorCollection();
	createPipeline();
}

void PraktikumsRenderer::cmdRender(size_t index, const VkCommandBuffer* commandBuffer) {
	descriptorCollection.cmdBind(index, commandBuffer);

	pipeline.cmdExecutePipeline(commandBuffer);

	storageImages.at(index).cmdCopyImage(commandBuffer, device->renderInfo.swapchainImages.at(index));
}

void PraktikumsRenderer::updateUniforms(size_t index) {
	globalData.backgroundColor = device->renderInfo.backgroundColor;
	globalData.lightPosition = device->renderInfo.lightPosition;

	globalDataBuffers.at(index).passData((void*) &globalData);

	for (size_t i = 0; i < objects.size(); ++i) {
		objects.at(i)->passBufferData(index);
		rtDataBuffers.at(index).passData(rtDataPtrs.at(i), i * GraphicsObject::getRTDataSize(), GraphicsObject::getRTDataSize());
	}
}

void PraktikumsRenderer::createTLAS() {
	for (GraphicsObject* obj: objects) {
		GraphicsObject::ObjectInfo info = obj->getObjectInfo();
		tlas.bufferProperties.blasInstances.push_back(info.instance);
		rtDataPtrs.push_back(info.dataPtr);
	}

	tlas.init();
}

void PraktikumsRenderer::createBuffers() {
	storageImages.bufferProperties.width = device->renderInfo.swapchainExtend.width;
	storageImages.bufferProperties.height = device->renderInfo.swapchainExtend.height;
	storageImages.bufferProperties.format = device->renderInfo.swapchainImageFormat;
	storageImages.bufferProperties.tiling = VK_IMAGE_TILING_OPTIMAL;
	storageImages.bufferProperties.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
	storageImages.bufferProperties.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	storageImages.bufferProperties.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
	storageImages.bufferProperties.layout = VK_IMAGE_LAYOUT_GENERAL;
	storageImages.bufferProperties.createImageView = true;
	storageImages.init();

	globalDataBuffers.bufferProperties.bufferSize = sizeof(PraktikumsRenderer::GlobalData);
	globalDataBuffers.bufferProperties.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	globalDataBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	globalDataBuffers.init();

	rtDataBuffers.bufferProperties.bufferSize = GraphicsObject::getRTDataSize() * rtDataPtrs.size();
	rtDataBuffers.bufferProperties.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	rtDataBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	rtDataBuffers.init();
}

void PraktikumsRenderer::createDescriptorCollection() {
	descriptorCollection.bufferDescriptors.resize(4);

	descriptorCollection.bufferDescriptors.at(0) = &tlas;
	descriptorCollection.bufferDescriptors.at(1) = &storageImages;
	descriptorCollection.bufferDescriptors.at(2) = &globalDataBuffers;
	descriptorCollection.bufferDescriptors.at(3) = &rtDataBuffers;

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
