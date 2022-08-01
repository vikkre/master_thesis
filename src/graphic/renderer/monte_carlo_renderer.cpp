#include "monte_carlo_renderer.h"


#define GLOBAL_BINDING_SET_INDEX 0

#define RGEN_SHADER "raygen_points.spv"
#define RCHIT_SHADER "closesthit.spv"
#define RMISS_SHADER "miss.spv"
#define RSHADOW_SHADER "shadow.spv"


MonteCarloRenderer::MonteCarloRenderer(Device* device)
:objects(), globalData(),
device(device), pipeline(device), tlas(device),
storageImages(), globalDataBuffers(), countBuffers(),
rtDataBuffers(), rtDataPtrs() {}

MonteCarloRenderer::~MonteCarloRenderer() {}

void MonteCarloRenderer::init() {
	createTLAS();
	createBuffers();
	createPipeline();
}

void MonteCarloRenderer::cmdRender(size_t index, const VkCommandBuffer* commandBuffer) {
	pipeline.cmdExecutePipeline(index, commandBuffer);

	storageImages.at(index).cmdCopyImage(commandBuffer, device->renderInfo.swapchainImages.at(index));
}

void MonteCarloRenderer::updateUniforms(size_t index) {
	globalData.backgroundColor = device->renderInfo.backgroundColor;
	globalData.lightPosition = device->renderInfo.lightPosition;

	globalDataBuffers.at(index).passData((void*) &globalData);


	uint32_t count = 0;
	countBuffers.at(index).getData((void*) &count);
	std::cout << "Hello count: " << count << std::endl;

	count = 0;
	countBuffers.at(index).passData((void*) &count);


	for (size_t i = 0; i < objects.size(); ++i) {
		objects.at(i)->passBufferData(index);
		rtDataBuffers.at(index).passData(rtDataPtrs.at(i), i * GraphicsObject::getRTDataSize(), GraphicsObject::getRTDataSize());
	}
}

void MonteCarloRenderer::createTLAS() {
	for (GraphicsObject* obj: objects) {
		GraphicsObject::ObjectInfo info = obj->getObjectInfo();
		tlas.blasInstances.push_back(info.instance);
		rtDataPtrs.push_back(info.dataPtr);
	}

	tlas.init();
}

void MonteCarloRenderer::createBuffers() {
	storageImages.reserve(device->renderInfo.swapchainImageCount);
	globalDataBuffers.reserve(device->renderInfo.swapchainImageCount);
	countBuffers.reserve(device->renderInfo.swapchainImageCount);
	rtDataBuffers.reserve(device->renderInfo.swapchainImageCount);

	for (size_t i = 0; i < device->renderInfo.swapchainImageCount; ++i) {
		storageImages.emplace_back(device);

		storageImages.at(i).width = device->renderInfo.swapchainExtend.width;
		storageImages.at(i).height = device->renderInfo.swapchainExtend.height;
		storageImages.at(i).format = device->renderInfo.swapchainImageFormat;
		storageImages.at(i).tiling = VK_IMAGE_TILING_OPTIMAL;
		storageImages.at(i).usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
		storageImages.at(i).properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		storageImages.at(i).aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
		storageImages.at(i).layout = VK_IMAGE_LAYOUT_GENERAL;
		storageImages.at(i).createImageView = true;

		storageImages.at(i).init();

		globalDataBuffers.emplace_back(device);

		globalDataBuffers.at(i).bufferSize = sizeof(MonteCarloRenderer::GlobalData);
		globalDataBuffers.at(i).usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		globalDataBuffers.at(i).properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		globalDataBuffers.at(i).init();

		countBuffers.emplace_back(device);

		countBuffers.at(i).bufferSize = sizeof(uint32_t);
		countBuffers.at(i).usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		countBuffers.at(i).properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		countBuffers.at(i).init();

		rtDataBuffers.emplace_back(device);

		rtDataBuffers.at(i).bufferSize = GraphicsObject::getRTDataSize() * rtDataPtrs.size();
		rtDataBuffers.at(i).usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		rtDataBuffers.at(i).properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		rtDataBuffers.at(i).init();
	}
}

void MonteCarloRenderer::createPipeline() {
	pipeline.raygenShaders.push_back(RGEN_SHADER);
	pipeline.missShaders.push_back(RMISS_SHADER);
	pipeline.hitShaders.push_back(RCHIT_SHADER);
	pipeline.missShaders.push_back(RSHADOW_SHADER);

	pipeline.bufferDescriptors.push_back(new SingleBufferDescriptor(&tlas, 0, VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR));
	// pipeline.bufferDescriptors.push_back(new MultiBufferDescriptor(MultiBufferDescriptor::vectorToBufferPointer(storageImages), 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR));
	pipeline.bufferDescriptors.push_back(new MultiBufferDescriptor(MultiBufferDescriptor::vectorToBufferPointer(globalDataBuffers), 2, VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR));
	pipeline.bufferDescriptors.push_back(new MultiBufferDescriptor(MultiBufferDescriptor::vectorToBufferPointer(countBuffers), 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR));
	pipeline.bufferDescriptors.push_back(new MultiBufferDescriptor(MultiBufferDescriptor::vectorToBufferPointer(rtDataBuffers), 3, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR));

	pipeline.width = 10000;
	// pipeline.width = device->renderInfo.swapchainExtend.width;
	// pipeline.height = device->renderInfo.swapchainExtend.height;

	pipeline.init();
}
