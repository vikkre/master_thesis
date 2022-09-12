#include "monte_carlo_renderer.h"


#define RCHIT_SHADER          "monte_carlo_closesthit.spv"
#define RMISS_SHADER          "monte_carlo_miss.spv"
#define LIGHT_GEN_RGEN_SHADER "monte_carlo_light_raygen.spv"
#define KD_COMP_SHADER        "monte_carlo_kd_comp.spv"
#define VISION_RGEN_SHADER    "monte_carlo_vision_raygen.spv"
#define FINAL_COMP_SHADER     "monte_carlo_final_comp.spv"


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
:device(device), descriptorCollection(device),
lightGenerationPipeline(device), kdPipeline(device), visionPipeline(device), finalRenderPipeline(device),
objDataPtrs(),
tlas(device), storageImagesRed(device), storageImagesGreen(device), storageImagesBlue(device),
globalDataBuffers(device), renderSettingsBuffers(device), countBuffers(device),
lightPointBuffers(device), kdBuffers(device), objDataBuffers(device) {}

MonteCarloRenderer::~MonteCarloRenderer() {}

void MonteCarloRenderer::init() {
	createTLAS();
	createBuffers();
	createDescriptorCollection();
	createLightGenerationPipeline();
	createKDPipeline();
	createVisionPipeline();
	createFinalRenderPipeline();
}

void MonteCarloRenderer::cmdRender(size_t index, VkCommandBuffer commandBuffer) {
	storageImagesRed.at(index).cmdClear(commandBuffer);
	storageImagesGreen.at(index).cmdClear(commandBuffer);
	storageImagesBlue.at(index).cmdClear(commandBuffer);

	descriptorCollection.cmdBind(index, commandBuffer);

	lightGenerationPipeline.cmdExecutePipeline(commandBuffer);

	Renderer::cmdPipelineBarrier(lightGenerationPipeline.getStageMask(), kdPipeline.getStageMask(), commandBuffer);

	kdPipeline.cmdExecutePipeline(commandBuffer);

	Renderer::cmdPipelineBarrier(kdPipeline.getStageMask(), visionPipeline.getStageMask(), commandBuffer);

	visionPipeline.cmdExecutePipeline(commandBuffer);

	Renderer::cmdPipelineBarrier(visionPipeline.getStageMask(), finalRenderPipeline.getStageMask(), commandBuffer);

	finalRenderPipeline.cmdExecutePipeline(commandBuffer);
}

void MonteCarloRenderer::updateUniforms(size_t index) {
	globalDataBuffers.at(index).passData((void*) &globalData);
	renderSettingsBuffers.at(index).passData((void*) &renderSettings);

	uint32_t count = 0;
	countBuffers.at(index).passData((void*) &count);

	for (size_t i = 0; i < objects.size(); ++i) {
		objects.at(i)->passBufferData(index);
		objDataBuffers.at(index).passData(objDataPtrs.at(i), i * GraphicsObject::getRTDataSize(), GraphicsObject::getRTDataSize());
	}
}

void MonteCarloRenderer::parseInput(const InputEntry& inputEntry) {
	renderSettings.backgroundColor = inputEntry.getVector<3, float>("backgroundColor");
	renderSettings.lightPosition = inputEntry.getVector<3, float>("lightPosition");
	renderSettings.lightRayCount = inputEntry.get<u_int32_t>("lightRayCount");
	renderSettings.lightJumpCount = inputEntry.get<u_int32_t>("lightJumpCount");
	renderSettings.visionJumpCount = inputEntry.get<u_int32_t>("visionJumpCount");
	renderSettings.collectionDistance = inputEntry.get<float>("collectionDistance");
	renderSettings.visionRayPerPixelCount = inputEntry.get<u_int32_t>("visionRayPerPixelCount");
	renderSettings.collectionDistanceShrinkFactor = inputEntry.get<float>("collectionDistanceShrinkFactor");
	renderSettings.lightCollectionCount = inputEntry.get<u_int32_t>("lightCollectionCount");
	renderSettings.useCountLightCollecton = inputEntry.get<u_int32_t>("useCountLightCollecton");
}

void MonteCarloRenderer::createTLAS() {
	for (GraphicsObject* obj: objects) {
		GraphicsObject::ObjectInfo info = obj->getObjectInfo();
		tlas.bufferProperties.blasInstances.push_back(info.instance);
		objDataPtrs.push_back(info.dataPtr);
	}

	tlas.init();
}

void MonteCarloRenderer::createBuffers() {
	ImageBuffer::Properties singleColorBufferProperties = outputImages->bufferProperties;
	singleColorBufferProperties.format = VK_FORMAT_R32_UINT;
	singleColorBufferProperties.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	storageImagesRed.bufferProperties = singleColorBufferProperties;
	storageImagesGreen.bufferProperties = singleColorBufferProperties;
	storageImagesBlue.bufferProperties = singleColorBufferProperties;

	storageImagesRed.init();
	storageImagesGreen.init();
	storageImagesBlue.init();

	globalDataBuffers.bufferProperties.bufferSize = sizeof(MonteCarloRenderer::GlobalData);
	globalDataBuffers.bufferProperties.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	globalDataBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	globalDataBuffers.init();

	renderSettingsBuffers.bufferProperties.bufferSize = sizeof(MonteCarloRenderer::RenderSettings);
	renderSettingsBuffers.bufferProperties.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	renderSettingsBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	renderSettingsBuffers.init();

	countBuffers.bufferProperties.bufferSize = sizeof(uint32_t);
	countBuffers.bufferProperties.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	countBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	countBuffers.init();

	lightPointBuffers.bufferProperties.bufferSize = sizeof(LightPoint) * renderSettings.lightRayCount * renderSettings.lightJumpCount;
	lightPointBuffers.bufferProperties.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	lightPointBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	lightPointBuffers.init();

	kdBuffers.bufferProperties.bufferSize = sizeof(KDData) * renderSettings.lightRayCount * renderSettings.lightJumpCount;
	kdBuffers.bufferProperties.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	kdBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	kdBuffers.init();

	objDataBuffers.bufferProperties.bufferSize = GraphicsObject::getRTDataSize() * objDataPtrs.size();
	objDataBuffers.bufferProperties.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	objDataBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	objDataBuffers.init();
}

void MonteCarloRenderer::createDescriptorCollection() {
	descriptorCollection.bufferDescriptors.resize(11);

	descriptorCollection.bufferDescriptors.at(0) = &tlas;
	descriptorCollection.bufferDescriptors.at(1) = &objDataBuffers;
	descriptorCollection.bufferDescriptors.at(2) = &globalDataBuffers;
	descriptorCollection.bufferDescriptors.at(3) = &renderSettingsBuffers;
	descriptorCollection.bufferDescriptors.at(4) = &countBuffers;
	descriptorCollection.bufferDescriptors.at(5) = &lightPointBuffers;
	descriptorCollection.bufferDescriptors.at(6) = &kdBuffers;
	descriptorCollection.bufferDescriptors.at(7) = &storageImagesRed;
	descriptorCollection.bufferDescriptors.at(8) = &storageImagesGreen;
	descriptorCollection.bufferDescriptors.at(9) = &storageImagesBlue;
	descriptorCollection.bufferDescriptors.at(10) = outputImages;

	descriptorCollection.init();
}

void MonteCarloRenderer::createLightGenerationPipeline() {
	lightGenerationPipeline.raygenShaders.push_back(LIGHT_GEN_RGEN_SHADER);
	lightGenerationPipeline.missShaders.push_back(RMISS_SHADER);
	lightGenerationPipeline.hitShaders.push_back(RCHIT_SHADER);

	lightGenerationPipeline.pipelineLayout = descriptorCollection.getPipelineLayout();

	lightGenerationPipeline.width = renderSettings.lightRayCount;

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
	visionPipeline.depth = renderSettings.visionRayPerPixelCount;

	visionPipeline.init();
}

void MonteCarloRenderer::createFinalRenderPipeline() {
	finalRenderPipeline.shaderPath = FINAL_COMP_SHADER;
	finalRenderPipeline.pipelineLayout = descriptorCollection.getPipelineLayout();

	finalRenderPipeline.x = device->renderInfo.swapchainExtend.width;
	finalRenderPipeline.y = device->renderInfo.swapchainExtend.height;

	finalRenderPipeline.init();
}
