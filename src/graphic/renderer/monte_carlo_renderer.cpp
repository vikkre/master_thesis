#include "monte_carlo_renderer.h"


#define GLOBAL_BINDING_SET_INDEX 0

#define LIGHT_GEN_RGEN_SHADER  "monte_carlo_light_raygen.spv"
#define LIGHT_GEN_RCHIT_SHADER "monte_carlo_light_closesthit.spv"
#define LIGHT_GEN_RMISS_SHADER "monte_carlo_light_miss.spv"
#define KD_RGEN_SHADER         "monte_carlo_kd_raygen.spv"

#define LIGHT_RAY_COUNT 10000
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
device(device), lightGenerationPipeline(device), kdPipeline(device), tlas(device),
storageImages(), globalDataBuffers(), countBuffers(), lightPointBuffers(), kdBuffers(),
rtDataBuffers(), rtDataPtrs() {}

MonteCarloRenderer::~MonteCarloRenderer() {}

void MonteCarloRenderer::init() {
	createTLAS();
	createBuffers();
	createLightGenerationPipeline();
	createKDPipeline();
}

void MonteCarloRenderer::cmdRender(size_t index, const VkCommandBuffer* commandBuffer) {
	lightGenerationPipeline.cmdExecutePipeline(index, commandBuffer);

	RayTracingPipeline::cmdRayTracingBarrier(commandBuffer);

	kdPipeline.cmdExecutePipeline(index, commandBuffer);

	storageImages.at(index).cmdCopyImage(commandBuffer, device->renderInfo.swapchainImages.at(index));
}

void MonteCarloRenderer::updateUniforms(size_t index) {
	globalData.backgroundColor = device->renderInfo.backgroundColor;
	globalData.lightPosition = device->renderInfo.lightPosition;
	globalData.lightJumpCount = LIGHT_JUMP_COUNT;
	globalData.visionJumpCount = VISION_JUMP_COUNT;

	globalDataBuffers.at(index).passData((void*) &globalData);


	uint32_t count = 0;
	countBuffers.at(index).getData((void*) &count);
	std::cout << "Count: " << count << std::endl;

	std::vector<LightPoint> lightPoints(LIGHT_RAY_COUNT);
	lightPointBuffers.at(index).getData((void*) lightPoints.data());

	std::vector<KDData> kddata(LIGHT_RAY_COUNT);
	kdBuffers.at(index).getData((void*) kddata.data());

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
	lightPointBuffers.reserve(device->renderInfo.swapchainImageCount);
	kdBuffers.reserve(device->renderInfo.swapchainImageCount);
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

		lightPointBuffers.emplace_back(device);

		lightPointBuffers.at(i).bufferSize = sizeof(LightPoint) * LIGHT_RAY_COUNT;
		lightPointBuffers.at(i).usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		lightPointBuffers.at(i).properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		lightPointBuffers.at(i).init();

		kdBuffers.emplace_back(device);

		kdBuffers.at(i).bufferSize = sizeof(KDData) * LIGHT_RAY_COUNT;
		kdBuffers.at(i).usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		kdBuffers.at(i).properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		kdBuffers.at(i).init();

		rtDataBuffers.emplace_back(device);

		rtDataBuffers.at(i).bufferSize = GraphicsObject::getRTDataSize() * rtDataPtrs.size();
		rtDataBuffers.at(i).usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		rtDataBuffers.at(i).properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		rtDataBuffers.at(i).init();
	}
}

void MonteCarloRenderer::createLightGenerationPipeline() {
	lightGenerationPipeline.raygenShaders.push_back(LIGHT_GEN_RGEN_SHADER);
	lightGenerationPipeline.missShaders.push_back(LIGHT_GEN_RMISS_SHADER);
	lightGenerationPipeline.hitShaders.push_back(LIGHT_GEN_RCHIT_SHADER);

	lightGenerationPipeline.bufferDescriptors.push_back(new SingleBufferDescriptor(&tlas, 0, VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR));
	lightGenerationPipeline.bufferDescriptors.push_back(new MultiBufferDescriptor(MultiBufferDescriptor::vectorToBufferPointer(rtDataBuffers), 1, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR));
	lightGenerationPipeline.bufferDescriptors.push_back(new MultiBufferDescriptor(MultiBufferDescriptor::vectorToBufferPointer(globalDataBuffers), 2, VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR));
	lightGenerationPipeline.bufferDescriptors.push_back(new MultiBufferDescriptor(MultiBufferDescriptor::vectorToBufferPointer(countBuffers), 3, VK_SHADER_STAGE_RAYGEN_BIT_KHR));
	lightGenerationPipeline.bufferDescriptors.push_back(new MultiBufferDescriptor(MultiBufferDescriptor::vectorToBufferPointer(lightPointBuffers), 4, VK_SHADER_STAGE_RAYGEN_BIT_KHR));

	lightGenerationPipeline.width = LIGHT_RAY_COUNT;

	lightGenerationPipeline.init();
}

void MonteCarloRenderer::createKDPipeline() {
	kdPipeline.raygenShaders.push_back(KD_RGEN_SHADER);
	kdPipeline.missShaders.push_back(LIGHT_GEN_RMISS_SHADER);
	kdPipeline.hitShaders.push_back(LIGHT_GEN_RCHIT_SHADER);

	kdPipeline.bufferDescriptors.push_back(new MultiBufferDescriptor(MultiBufferDescriptor::vectorToBufferPointer(countBuffers), 0, VK_SHADER_STAGE_RAYGEN_BIT_KHR));
	kdPipeline.bufferDescriptors.push_back(new MultiBufferDescriptor(MultiBufferDescriptor::vectorToBufferPointer(lightPointBuffers), 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR));
	kdPipeline.bufferDescriptors.push_back(new MultiBufferDescriptor(MultiBufferDescriptor::vectorToBufferPointer(kdBuffers), 2, VK_SHADER_STAGE_RAYGEN_BIT_KHR));

	kdPipeline.init();
}
