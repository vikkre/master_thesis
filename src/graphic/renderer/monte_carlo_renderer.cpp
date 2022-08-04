#include "monte_carlo_renderer.h"


#define LIGHT_GEN_RGEN_SHADER  "monte_carlo_light_raygen.spv"
#define LIGHT_GEN_RCHIT_SHADER "monte_carlo_light_closesthit.spv"
#define LIGHT_GEN_RMISS_SHADER "monte_carlo_light_miss.spv"
#define KD_RGEN_SHADER         "monte_carlo_kd_raygen.spv"
#define VISION_RGEN_SHADER     "monte_carlo_vision_raygen.spv"
#define VISION_RCHIT_SHADER    "monte_carlo_vision_closesthit.spv"
#define VISION_RMISS_SHADER    "monte_carlo_vision_miss.spv"

#define LIGHT_RAY_COUNT 2500
#define VISION_RAY_COUNT_PER_PIXEL 1
#define LIGHT_JUMP_COUNT 1
#define VISION_JUMP_COUNT 2


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
lightGenerationPipeline(device), kdPipeline(device), visionPipeline(device),
rtDataPtrs(),
tlas(device), storageImages(device), globalDataBuffers(device), countBuffers(device),
lightPointBuffers(device), kdBuffers(device), rtDataBuffers(device) {}

MonteCarloRenderer::~MonteCarloRenderer() {}

void MonteCarloRenderer::init() {
	createTLAS();
	createBuffers();
	createDescriptorCollection();
	createLightGenerationPipeline();
	createKDPipeline();
	createVisionPipeline();
}

void MonteCarloRenderer::cmdRender(size_t index, const VkCommandBuffer* commandBuffer) {
	descriptorCollection.cmdBind(index, commandBuffer);

	lightGenerationPipeline.cmdExecutePipeline(commandBuffer);

	RayTracingPipeline::cmdRayTracingBarrier(commandBuffer);

	kdPipeline.cmdExecutePipeline(commandBuffer);

	RayTracingPipeline::cmdRayTracingBarrier(commandBuffer);

	visionPipeline.cmdExecutePipeline(commandBuffer);

	storageImages.at(index).cmdCopyImage(commandBuffer, device->renderInfo.swapchainImages.at(index));
}

void MonteCarloRenderer::updateUniforms(size_t index) {
	globalData.backgroundColor = device->renderInfo.backgroundColor;
	globalData.lightPosition = device->renderInfo.lightPosition;
	globalData.lightJumpCount = LIGHT_JUMP_COUNT;
	globalData.visionJumpCount = VISION_JUMP_COUNT;

	globalDataBuffers.at(index).passData((void*) &globalData);


	uint32_t count = 0;
	// countBuffers.at(index).getData((void*) &count);
	// std::cout << "Count: " << count << std::endl;

	// std::vector<LightPoint> lightPoints(LIGHT_RAY_COUNT);
	// lightPointBuffers.at(index).getData((void*) lightPoints.data());

	// std::vector<KDData> kddata(LIGHT_RAY_COUNT);
	// kdBuffers.at(index).getData((void*) kddata.data());

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
		tlas.bufferProperties.blasInstances.push_back(info.instance);
		rtDataPtrs.push_back(info.dataPtr);
	}

	tlas.init();
}

void MonteCarloRenderer::createBuffers() {
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

	globalDataBuffers.bufferProperties.bufferSize = sizeof(MonteCarloRenderer::GlobalData);
	globalDataBuffers.bufferProperties.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	globalDataBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	globalDataBuffers.init();

	countBuffers.bufferProperties.bufferSize = sizeof(uint32_t);
	countBuffers.bufferProperties.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	countBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	countBuffers.init();

	lightPointBuffers.bufferProperties.bufferSize = sizeof(LightPoint) * LIGHT_RAY_COUNT;
	lightPointBuffers.bufferProperties.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	lightPointBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	lightPointBuffers.init();

	kdBuffers.bufferProperties.bufferSize = sizeof(KDData) * LIGHT_RAY_COUNT;
	kdBuffers.bufferProperties.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	kdBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	kdBuffers.init();

	rtDataBuffers.bufferProperties.bufferSize = GraphicsObject::getRTDataSize() * rtDataPtrs.size();
	rtDataBuffers.bufferProperties.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	rtDataBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	rtDataBuffers.init();
}

void MonteCarloRenderer::createDescriptorCollection() {
	descriptorCollection.bufferDescriptors.resize(7);

	descriptorCollection.bufferDescriptors.at(0) = &tlas;
	descriptorCollection.bufferDescriptors.at(1) = &rtDataBuffers;
	descriptorCollection.bufferDescriptors.at(2) = &globalDataBuffers;
	descriptorCollection.bufferDescriptors.at(3) = &countBuffers;
	descriptorCollection.bufferDescriptors.at(4) = &lightPointBuffers;
	descriptorCollection.bufferDescriptors.at(5) = &kdBuffers;
	descriptorCollection.bufferDescriptors.at(6) = &storageImages;

	descriptorCollection.init();
}

void MonteCarloRenderer::createLightGenerationPipeline() {
	lightGenerationPipeline.raygenShaders.push_back(LIGHT_GEN_RGEN_SHADER);
	lightGenerationPipeline.missShaders.push_back(LIGHT_GEN_RMISS_SHADER);
	lightGenerationPipeline.hitShaders.push_back(LIGHT_GEN_RCHIT_SHADER);

	lightGenerationPipeline.pipelineLayout = descriptorCollection.getPipelineLayout();

	lightGenerationPipeline.width = LIGHT_RAY_COUNT;

	lightGenerationPipeline.init();
}

void MonteCarloRenderer::createKDPipeline() {
	kdPipeline.raygenShaders.push_back(KD_RGEN_SHADER);
	kdPipeline.missShaders.push_back(LIGHT_GEN_RMISS_SHADER);
	kdPipeline.hitShaders.push_back(LIGHT_GEN_RCHIT_SHADER);

	kdPipeline.pipelineLayout = descriptorCollection.getPipelineLayout();

	kdPipeline.init();
}

void MonteCarloRenderer::createVisionPipeline() {
	visionPipeline.raygenShaders.push_back(VISION_RGEN_SHADER);
	visionPipeline.missShaders.push_back(VISION_RMISS_SHADER);
	visionPipeline.hitShaders.push_back(VISION_RCHIT_SHADER);

	visionPipeline.pipelineLayout = descriptorCollection.getPipelineLayout();

	visionPipeline.width = device->renderInfo.swapchainExtend.width;
	visionPipeline.height = device->renderInfo.swapchainExtend.height;
	visionPipeline.depth = VISION_RAY_COUNT_PER_PIXEL;

	visionPipeline.init();
}
