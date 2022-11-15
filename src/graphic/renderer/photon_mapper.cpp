#include "photon_mapper.h"


#define LIGHT_GEN_RGEN_SHADER "photon_mapper_light_raygen.spv"
#define KD_COMP_SHADER        "photon_mapper_kd_comp.spv"
#define VISION_RGEN_SHADER    "photon_mapper_vision_raygen.spv"
#define FINAL_COMP_SHADER     "photon_mapper_final_comp.spv"


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


PhotonMapper::PhotonMapper(Device* device)
:Renderer(device), device(device), descriptorCollection(device),
lightGenerationPipeline(device), kdPipeline(device), visionPipeline(device), finalRenderPipeline(device),
storageImagesRed(device), storageImagesGreen(device), storageImagesBlue(device),
renderSettingsBuffers(device), countBuffers(device),
lightPointBuffers(device), kdBuffers(device)
{}

PhotonMapper::~PhotonMapper() {}

void PhotonMapper::initRenderer() {
	createBuffers();
	createDescriptorCollection();
	createPipelineLayout();
	createLightGenerationPipeline();
	createKDPipeline();
	createVisionPipeline();
	createFinalRenderPipeline();
}

void PhotonMapper::cmdRenderFrame(size_t index, VkCommandBuffer commandBuffer) {
	storageImagesRed.at(index).cmdClear(commandBuffer);
	storageImagesGreen.at(index).cmdClear(commandBuffer);
	storageImagesBlue.at(index).cmdClear(commandBuffer);

	descriptorCollection.cmdBind(index, commandBuffer, getPipelineLayout());

	lightGenerationPipeline.cmdExecutePipeline(commandBuffer);

	Renderer::cmdPipelineBarrier(lightGenerationPipeline.getStageMask(), kdPipeline.getStageMask(), commandBuffer);

	kdPipeline.cmdExecutePipeline(commandBuffer);

	Renderer::cmdPipelineBarrier(kdPipeline.getStageMask(), visionPipeline.getStageMask(), commandBuffer);

	visionPipeline.cmdExecutePipeline(commandBuffer);

	Renderer::cmdPipelineBarrier(visionPipeline.getStageMask(), finalRenderPipeline.getStageMask(), commandBuffer);

	finalRenderPipeline.cmdExecutePipeline(commandBuffer);
}

void PhotonMapper::updateRendererUniforms(size_t index) {
	renderSettingsBuffers.at(index).passData((void*) &renderSettings);

	uint32_t count = 0;
	countBuffers.at(index).passData((void*) &count);
}

void PhotonMapper::parseRendererInput(const InputEntry& inputEntry) {
	renderSettings.lightRayCount = inputEntry.get<u_int32_t>("lightRayCount");
	renderSettings.lightJumpCount = inputEntry.get<u_int32_t>("lightJumpCount");
	renderSettings.visionJumpCount = inputEntry.get<u_int32_t>("visionJumpCount");
	renderSettings.collectionDistance = inputEntry.get<float>("collectionDistance");
	renderSettings.visionRayPerPixelCount = inputEntry.get<u_int32_t>("visionRayPerPixelCount");
	renderSettings.collectionDistanceShrinkFactor = inputEntry.get<float>("collectionDistanceShrinkFactor");
	renderSettings.lightCollectionCount = inputEntry.get<u_int32_t>("lightCollectionCount");
	renderSettings.useCountLightCollecton = inputEntry.get<u_int32_t>("useCountLightCollecton");
}

void PhotonMapper::createBuffers() {
	ImageBuffer::Properties singleColorBufferProperties = outputImages->bufferProperties;
	singleColorBufferProperties.format = VK_FORMAT_R32_UINT;
	singleColorBufferProperties.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	storageImagesRed.bufferProperties = singleColorBufferProperties;
	storageImagesGreen.bufferProperties = singleColorBufferProperties;
	storageImagesBlue.bufferProperties = singleColorBufferProperties;

	storageImagesRed.init();
	storageImagesGreen.init();
	storageImagesBlue.init();

	renderSettingsBuffers.bufferProperties.bufferSize = sizeof(PhotonMapper::RenderSettings);
	renderSettingsBuffers.bufferProperties.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	renderSettingsBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	renderSettingsBuffers.init();

	countBuffers.bufferProperties.bufferSize = sizeof(uint32_t);
	countBuffers.bufferProperties.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	countBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	countBuffers.init();

	lightPointBuffers.bufferProperties.bufferSize = sizeof(LightPoint) * renderSettings.lightRayCount * renderSettings.lightJumpCount;
	lightPointBuffers.bufferProperties.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	lightPointBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	lightPointBuffers.init();

	kdBuffers.bufferProperties.bufferSize = sizeof(KDData) * renderSettings.lightRayCount * renderSettings.lightJumpCount;
	kdBuffers.bufferProperties.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	kdBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	kdBuffers.init();
}

void PhotonMapper::createDescriptorCollection() {
	descriptorCollection.bufferDescriptors.resize(8);

	descriptorCollection.bufferDescriptors.at(0) = &renderSettingsBuffers;
	descriptorCollection.bufferDescriptors.at(1) = &countBuffers;
	descriptorCollection.bufferDescriptors.at(2) = &lightPointBuffers;
	descriptorCollection.bufferDescriptors.at(3) = &kdBuffers;
	descriptorCollection.bufferDescriptors.at(4) = &storageImagesRed;
	descriptorCollection.bufferDescriptors.at(5) = &storageImagesGreen;
	descriptorCollection.bufferDescriptors.at(6) = &storageImagesBlue;
	descriptorCollection.bufferDescriptors.at(7) = outputImages;

	descriptorCollection.init();

	descriptors.push_back(&descriptorCollection);
}

void PhotonMapper::createLightGenerationPipeline() {
	lightGenerationPipeline.raygenShaders.push_back(LIGHT_GEN_RGEN_SHADER);
	lightGenerationPipeline.missShaders = Renderer::RMISS_SHADERS;
	lightGenerationPipeline.hitShaders = Renderer::RCHIT_SHADERS;

	lightGenerationPipeline.pipelineLayout = getPipelineLayout();

	lightGenerationPipeline.width = renderSettings.lightRayCount;

	lightGenerationPipeline.init();
}

void PhotonMapper::createKDPipeline() {
	kdPipeline.shaderPath = KD_COMP_SHADER;
	
	kdPipeline.pipelineLayout = getPipelineLayout();

	kdPipeline.init();
}

void PhotonMapper::createVisionPipeline() {
	visionPipeline.raygenShaders.push_back(VISION_RGEN_SHADER);
	visionPipeline.missShaders = Renderer::RMISS_SHADERS;
	visionPipeline.hitShaders = Renderer::RCHIT_SHADERS;

	visionPipeline.pipelineLayout = getPipelineLayout();

	visionPipeline.width = device->renderInfo.swapchainExtend.width;
	visionPipeline.height = device->renderInfo.swapchainExtend.height;
	visionPipeline.depth = renderSettings.visionRayPerPixelCount;

	visionPipeline.init();
}

void PhotonMapper::createFinalRenderPipeline() {
	finalRenderPipeline.shaderPath = FINAL_COMP_SHADER;
	
	finalRenderPipeline.pipelineLayout = getPipelineLayout();

	finalRenderPipeline.x = device->renderInfo.swapchainExtend.width;
	finalRenderPipeline.y = device->renderInfo.swapchainExtend.height;

	finalRenderPipeline.init();
}
