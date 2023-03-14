#include "bitterli2020_bidirectional_path_tracer.h"


#define RESERVOIR_RGEN_SHADER "bitterli2020_bidirectional_path_tracer_reservoir_raygen.spv"
#define RESULT_RGEN_SHADER    "bitterli2020_bidirectional_path_tracer_result_raygen.spv"


struct LightSourcePoint {
	Vector3f pos;
	Vector3f normal;
	Vector3f color;
	float lightStrength;
};

struct Sample {
	LightSourcePoint lsp;
	float weight;
};

struct Reservoir {
	Sample y;
	float w_sum;
	u_int32_t M;
	float W;
};

struct RayPayload {
	Vector3f pos;
	Vector3f normal;
	Vector3f color;
	u_int32_t hit;
	u_int32_t lightSource;
};


Bitterli2020BidirectionalPathTracer::Bitterli2020BidirectionalPathTracer(Device* device)
:Renderer(device), device(device), descriptorCollection(device),
reservoirPipeline(device), resultPipeline(device),
renderSettingsBuffers(device), rayPayloadsBuffers(device), spatialReservoirsBuffers(device) {}

Bitterli2020BidirectionalPathTracer::~Bitterli2020BidirectionalPathTracer() {}

void Bitterli2020BidirectionalPathTracer::initRenderer() {
	createBuffers();
	createDescriptorCollection();
	createPipelineLayout();
	createReservoirPipeline();
	createResultPipeline();
}

void Bitterli2020BidirectionalPathTracer::cmdRenderFrame(size_t index, VkCommandBuffer commandBuffer) {
	descriptorCollection.cmdBind(index, commandBuffer, getPipelineLayout());

	reservoirPipeline.cmdExecutePipeline(commandBuffer);

	Renderer::cmdPipelineBarrier(reservoirPipeline.getStageMask(), resultPipeline.getStageMask(), commandBuffer);

	resultPipeline.cmdExecutePipeline(commandBuffer);
}

void Bitterli2020BidirectionalPathTracer::updateRendererUniforms(size_t index) {
	renderSettingsBuffers.at(index).passData((void*) &renderSettings);
}

void Bitterli2020BidirectionalPathTracer::parseRendererInput(const InputEntry& inputEntry) {
	renderSettings.visionJumpCount = inputEntry.get<u_int32_t>("visionJumpCount");
	renderSettings.lightJumpCount  = inputEntry.get<u_int32_t>("lightJumpCount");
	renderSettings.maxDepth        = inputEntry.get<u_int32_t>("maxDepth");
	renderSettings.candidateCount  = inputEntry.get<u_int32_t>("candidateCount");
	renderSettings.sampleCount     = inputEntry.get<u_int32_t>("sampleCount");
}

void Bitterli2020BidirectionalPathTracer::createBuffers() {
	renderSettingsBuffers.bufferProperties.bufferSize = sizeof(Bitterli2020BidirectionalPathTracer::RenderSettings);
	renderSettingsBuffers.bufferProperties.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	renderSettingsBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	renderSettingsBuffers.init();

	unsigned int bufferSize = device->renderInfo.swapchainExtend.width * device->renderInfo.swapchainExtend.height;

	rayPayloadsBuffers.bufferProperties.bufferSize = sizeof(RayPayload) * bufferSize;
	rayPayloadsBuffers.bufferProperties.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	rayPayloadsBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	rayPayloadsBuffers.init();

	spatialReservoirsBuffers.bufferProperties.bufferSize = sizeof(Reservoir) * renderSettings.sampleCount * bufferSize;
	spatialReservoirsBuffers.bufferProperties.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	spatialReservoirsBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	spatialReservoirsBuffers.init();
}

void Bitterli2020BidirectionalPathTracer::createDescriptorCollection() {
	descriptorCollection.addBuffer(0, &renderSettingsBuffers);
	descriptorCollection.addBuffer(1, outputImages);
	descriptorCollection.addBuffer(2, &rayPayloadsBuffers);
	descriptorCollection.addBuffer(3, &spatialReservoirsBuffers);

	descriptorCollection.init();

	descriptors.push_back(&descriptorCollection);
}

void Bitterli2020BidirectionalPathTracer::createReservoirPipeline() {
	reservoirPipeline.raygenShaders.push_back(RESERVOIR_RGEN_SHADER);
	reservoirPipeline.missShaders = Renderer::RMISS_SHADERS;
	reservoirPipeline.hitShaders = Renderer::RCHIT_SHADERS;

	reservoirPipeline.pipelineLayout = getPipelineLayout();

	reservoirPipeline.width = device->renderInfo.swapchainExtend.width;
	reservoirPipeline.height = device->renderInfo.swapchainExtend.height;

	reservoirPipeline.init();
}

void Bitterli2020BidirectionalPathTracer::createResultPipeline() {
	resultPipeline.raygenShaders.push_back(RESULT_RGEN_SHADER);
	resultPipeline.missShaders = Renderer::RMISS_SHADERS;
	resultPipeline.hitShaders = Renderer::RCHIT_SHADERS;

	resultPipeline.pipelineLayout = getPipelineLayout();

	resultPipeline.width = device->renderInfo.swapchainExtend.width;
	resultPipeline.height = device->renderInfo.swapchainExtend.height;

	resultPipeline.init();
}
