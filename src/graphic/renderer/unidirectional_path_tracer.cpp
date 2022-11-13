#include "unidirectional_path_tracer.h"


#define VISION_RGEN_SHADER    "unidirectional_path_tracer_vision_raygen.spv"
#define FINAL_COMP_SHADER     "unidirectional_path_tracer_final_comp.spv"


UnidirectionalPathTracer::UnidirectionalPathTracer(Device* device)
:Renderer(device), device(device), descriptorCollection(device),
visionPipeline(device), finalRenderPipeline(device),
storageImagesRed(device), storageImagesGreen(device), storageImagesBlue(device),
renderSettingsBuffers(device) {}

UnidirectionalPathTracer::~UnidirectionalPathTracer() {}

void UnidirectionalPathTracer::initRenderer() {
	createBuffers();
	createDescriptorCollection();
	createPipelineLayout();
	createVisionPipeline();
	createFinalRenderPipeline();
}

void UnidirectionalPathTracer::cmdRenderFrame(size_t index, VkCommandBuffer commandBuffer) {
	storageImagesRed.at(index).cmdClear(commandBuffer);
	storageImagesGreen.at(index).cmdClear(commandBuffer);
	storageImagesBlue.at(index).cmdClear(commandBuffer);

	descriptorCollection.cmdBind(index, commandBuffer, getPipelineLayout());

	visionPipeline.cmdExecutePipeline(commandBuffer);

	Renderer::cmdPipelineBarrier(visionPipeline.getStageMask(), finalRenderPipeline.getStageMask(), commandBuffer);

	finalRenderPipeline.cmdExecutePipeline(commandBuffer);
}

void UnidirectionalPathTracer::updateRendererUniforms(size_t index) {
	renderSettingsBuffers.at(index).passData((void*) &renderSettings);
}

void UnidirectionalPathTracer::parseRendererInput(const InputEntry& inputEntry) {
	renderSettings.visionJumpCount = inputEntry.get<u_int32_t>("visionJumpCount");
	renderSettings.visionRayPerPixelCount = inputEntry.get<u_int32_t>("visionRayPerPixelCount");
}

void UnidirectionalPathTracer::createBuffers() {
	ImageBuffer::Properties singleColorBufferProperties = outputImages->bufferProperties;
	singleColorBufferProperties.format = VK_FORMAT_R32_UINT;
	singleColorBufferProperties.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	storageImagesRed.bufferProperties = singleColorBufferProperties;
	storageImagesGreen.bufferProperties = singleColorBufferProperties;
	storageImagesBlue.bufferProperties = singleColorBufferProperties;

	storageImagesRed.init();
	storageImagesGreen.init();
	storageImagesBlue.init();

	renderSettingsBuffers.bufferProperties.bufferSize = sizeof(UnidirectionalPathTracer::RenderSettings);
	renderSettingsBuffers.bufferProperties.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	renderSettingsBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	renderSettingsBuffers.init();
}

void UnidirectionalPathTracer::createDescriptorCollection() {
	descriptorCollection.bufferDescriptors.resize(5);

	descriptorCollection.bufferDescriptors.at(0) = &renderSettingsBuffers;
	descriptorCollection.bufferDescriptors.at(1) = &storageImagesRed;
	descriptorCollection.bufferDescriptors.at(2) = &storageImagesGreen;
	descriptorCollection.bufferDescriptors.at(3) = &storageImagesBlue;
	descriptorCollection.bufferDescriptors.at(4) = outputImages;

	descriptorCollection.init();

	descriptors.push_back(&descriptorCollection);
}

void UnidirectionalPathTracer::createVisionPipeline() {
	visionPipeline.raygenShaders.push_back(VISION_RGEN_SHADER);
	visionPipeline.missShaders = Renderer::RMISS_SHADERS;
	visionPipeline.hitShaders = Renderer::RCHIT_SHADERS;

	visionPipeline.pipelineLayout = getPipelineLayout();

	visionPipeline.width = device->renderInfo.swapchainExtend.width;
	visionPipeline.height = device->renderInfo.swapchainExtend.height;
	visionPipeline.depth = renderSettings.visionRayPerPixelCount;

	visionPipeline.init();
}

void UnidirectionalPathTracer::createFinalRenderPipeline() {
	finalRenderPipeline.shaderPath = FINAL_COMP_SHADER;
	
	finalRenderPipeline.pipelineLayout = getPipelineLayout();

	finalRenderPipeline.x = device->renderInfo.swapchainExtend.width;
	finalRenderPipeline.y = device->renderInfo.swapchainExtend.height;

	finalRenderPipeline.init();
}
