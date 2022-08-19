#include "monte_carlo_renderer.h"


#define RCHIT_SHADER "monte_carlo_closesthit.spv"
#define RMISS_SHADER "monte_carlo_miss.spv"
#define LIGHT_GEN_RGEN_SHADER  "monte_carlo_light_raygen.spv"
#define KD_COMP_SHADER         "monte_carlo_kd_comp.spv"
#define VISION_RGEN_SHADER     "monte_carlo_vision_raygen.spv"
#define FINAL_COMP_SHADER      "monte_carlo_final_comp.spv"
#define DENOISE_COMP_SHADER    "monte_carlo_denoise_comp.spv"

#define LIGHT_RAY_COUNT 250
#define VISION_RAY_COUNT_PER_PIXEL 30
#define LIGHT_JUMP_COUNT 5
#define VISION_JUMP_COUNT 5
#define LIGHT_COLLECTION_DISTANCE 0.4f


struct LightPoint {
	Vector3f pos;
	Vector3f color;
};

struct KDData {
	Vector3f pos;
	Vector3f color;
	uint32_t direction;
	int32_t leftIndex;
	int32_t rightIndex;
};


MonteCarloRenderer::MonteCarloRenderer(Device* device)
:objects(), globalData(),
device(device), descriptorCollection(device),
lightGenerationPipeline(device), kdPipeline(device), visionPipeline(device), finalRenderPipeline(device), denoisePipeline(device), 
rtDataPtrs(),
tlas(device), storageImagesRed(device), storageImagesGreen(device), storageImagesBlue(device),renderedImages(device), finalImages(device),
globalDataBuffers(device), countBuffers(device),
lightPointBuffers(device), kdBuffers(device), rtDataBuffers(device) {}

MonteCarloRenderer::~MonteCarloRenderer() {}

void MonteCarloRenderer::init() {
	createTLAS();
	createBuffers();
	createDescriptorCollection();
	createLightGenerationPipeline();
	createKDPipeline();
	createVisionPipeline();
	createFinalRenderPipeline();
	createDenoisePipeline();
}

void MonteCarloRenderer::cmdRender(size_t index, VkCommandBuffer commandBuffer) {
	storageImagesRed.at(index).cmdClear(commandBuffer);
	storageImagesGreen.at(index).cmdClear(commandBuffer);
	storageImagesBlue.at(index).cmdClear(commandBuffer);

	descriptorCollection.cmdBind(index, commandBuffer);

	lightGenerationPipeline.cmdExecutePipeline(commandBuffer);

	RayTracingPipeline::cmdRayTracingBarrier(commandBuffer);

	kdPipeline.cmdExecutePipeline(commandBuffer);

	RayTracingPipeline::cmdRayTracingBarrier(commandBuffer);

	visionPipeline.cmdExecutePipeline(commandBuffer);

	RayTracingPipeline::cmdRayTracingBarrier(commandBuffer);

	finalRenderPipeline.cmdExecutePipeline(commandBuffer);

	RayTracingPipeline::cmdRayTracingBarrier(commandBuffer);

	denoisePipeline.cmdExecutePipeline(commandBuffer);

	finalImages.at(index).cmdCopyImage(commandBuffer, device->renderInfo.swapchainImages.at(index));

	RayTracingPipeline::cmdRayTracingBarrier(commandBuffer);
}

void MonteCarloRenderer::updateUniforms(size_t index) {
	globalData.backgroundColor = device->renderInfo.backgroundColor;
	globalData.lightPosition = device->renderInfo.lightPosition;
	globalData.lightJumpCount = LIGHT_JUMP_COUNT;
	globalData.visionJumpCount = VISION_JUMP_COUNT;
	globalData.collectionDistance = LIGHT_COLLECTION_DISTANCE;
	globalData.rayPerPixelCount = VISION_RAY_COUNT_PER_PIXEL;

	globalDataBuffers.at(index).passData((void*) &globalData);

	uint32_t count = 0;
	countBuffers.at(index).passData((void*) &count);


	for (size_t i = 0; i < objects.size(); ++i) {
		objects.at(i)->passBufferData(index);
		rtDataBuffers.at(index).passData(rtDataPtrs.at(i), i * GraphicsObject::getRTDataSize(), GraphicsObject::getRTDataSize());
	}
}

void MonteCarloRenderer::createTLAS() {
	for (GraphicsObject* obj: objects) {
		GraphicsObject::ObjectInfo info = obj->getObjectInfo();
		tlas.bufferProperties.blasInstances.push_back(info.instance);
		rtDataPtrs.push_back(info.dataPtr);
	}

	tlas.init();
}

void MonteCarloRenderer::createBuffers() {
	storageImagesRed.bufferProperties.width = device->renderInfo.swapchainExtend.width;
	storageImagesRed.bufferProperties.height = device->renderInfo.swapchainExtend.height;
	storageImagesRed.bufferProperties.format = VK_FORMAT_R32_UINT;
	storageImagesRed.bufferProperties.tiling = VK_IMAGE_TILING_OPTIMAL;
	storageImagesRed.bufferProperties.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
	storageImagesRed.bufferProperties.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	storageImagesRed.bufferProperties.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
	storageImagesRed.bufferProperties.layout = VK_IMAGE_LAYOUT_GENERAL;
	storageImagesRed.bufferProperties.createImageView = true;
	storageImagesRed.init();

	storageImagesGreen.bufferProperties = storageImagesRed.bufferProperties;
	storageImagesGreen.init();
	storageImagesBlue.bufferProperties = storageImagesRed.bufferProperties;
	storageImagesBlue.init();

	renderedImages.bufferProperties.width = device->renderInfo.swapchainExtend.width;
	renderedImages.bufferProperties.height = device->renderInfo.swapchainExtend.height;
	renderedImages.bufferProperties.format = device->renderInfo.swapchainImageFormat;
	renderedImages.bufferProperties.tiling = VK_IMAGE_TILING_OPTIMAL;
	renderedImages.bufferProperties.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
	renderedImages.bufferProperties.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	renderedImages.bufferProperties.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
	renderedImages.bufferProperties.layout = VK_IMAGE_LAYOUT_GENERAL;
	renderedImages.bufferProperties.createImageView = true;
	renderedImages.init();

	finalImages.bufferProperties = renderedImages.bufferProperties;
	finalImages.init();

	globalDataBuffers.bufferProperties.bufferSize = sizeof(MonteCarloRenderer::GlobalData);
	globalDataBuffers.bufferProperties.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	globalDataBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	globalDataBuffers.init();

	countBuffers.bufferProperties.bufferSize = sizeof(uint32_t);
	countBuffers.bufferProperties.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	countBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	countBuffers.init();

	lightPointBuffers.bufferProperties.bufferSize = sizeof(LightPoint) * LIGHT_RAY_COUNT * LIGHT_JUMP_COUNT;
	lightPointBuffers.bufferProperties.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	lightPointBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	lightPointBuffers.init();

	kdBuffers.bufferProperties.bufferSize = sizeof(KDData) * LIGHT_RAY_COUNT * LIGHT_JUMP_COUNT;
	kdBuffers.bufferProperties.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	kdBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	kdBuffers.init();

	rtDataBuffers.bufferProperties.bufferSize = GraphicsObject::getRTDataSize() * rtDataPtrs.size();
	rtDataBuffers.bufferProperties.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	rtDataBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	rtDataBuffers.init();
}

void MonteCarloRenderer::createDescriptorCollection() {
	descriptorCollection.bufferDescriptors.resize(11);

	descriptorCollection.bufferDescriptors.at(0) = &tlas;
	descriptorCollection.bufferDescriptors.at(1) = &rtDataBuffers;
	descriptorCollection.bufferDescriptors.at(2) = &globalDataBuffers;
	descriptorCollection.bufferDescriptors.at(3) = &countBuffers;
	descriptorCollection.bufferDescriptors.at(4) = &lightPointBuffers;
	descriptorCollection.bufferDescriptors.at(5) = &kdBuffers;
	descriptorCollection.bufferDescriptors.at(6) = &storageImagesRed;
	descriptorCollection.bufferDescriptors.at(7) = &storageImagesGreen;
	descriptorCollection.bufferDescriptors.at(8) = &storageImagesBlue;
	descriptorCollection.bufferDescriptors.at(9) = &renderedImages;
	descriptorCollection.bufferDescriptors.at(10) = &finalImages;

	descriptorCollection.init();
}

void MonteCarloRenderer::createLightGenerationPipeline() {
	lightGenerationPipeline.raygenShaders.push_back(LIGHT_GEN_RGEN_SHADER);
	lightGenerationPipeline.missShaders.push_back(RMISS_SHADER);
	lightGenerationPipeline.hitShaders.push_back(RCHIT_SHADER);

	lightGenerationPipeline.pipelineLayout = descriptorCollection.getPipelineLayout();

	lightGenerationPipeline.width = LIGHT_RAY_COUNT;

	lightGenerationPipeline.init();
}

void MonteCarloRenderer::createKDPipeline() {
	kdPipeline.shaderPath = KD_COMP_SHADER;
	kdPipeline.pipelineLayout = descriptorCollection.getPipelineLayout();
	kdPipeline.init();
}

void MonteCarloRenderer::createVisionPipeline() {
	visionPipeline.raygenShaders.push_back(VISION_RGEN_SHADER);
	visionPipeline.missShaders.push_back(RMISS_SHADER);
	visionPipeline.hitShaders.push_back(RCHIT_SHADER);

	visionPipeline.pipelineLayout = descriptorCollection.getPipelineLayout();

	visionPipeline.width = device->renderInfo.swapchainExtend.width;
	visionPipeline.height = device->renderInfo.swapchainExtend.height;
	visionPipeline.depth = VISION_RAY_COUNT_PER_PIXEL;

	visionPipeline.init();
}

void MonteCarloRenderer::createFinalRenderPipeline() {
	finalRenderPipeline.shaderPath = FINAL_COMP_SHADER;
	finalRenderPipeline.pipelineLayout = descriptorCollection.getPipelineLayout();

	finalRenderPipeline.x = device->renderInfo.swapchainExtend.width;
	finalRenderPipeline.y = device->renderInfo.swapchainExtend.height;

	finalRenderPipeline.init();
}

void MonteCarloRenderer::createDenoisePipeline() {
	denoisePipeline.shaderPath = DENOISE_COMP_SHADER;
	denoisePipeline.pipelineLayout = descriptorCollection.getPipelineLayout();

	denoisePipeline.x = device->renderInfo.swapchainExtend.width;
	denoisePipeline.y = device->renderInfo.swapchainExtend.height;

	denoisePipeline.init();
}
