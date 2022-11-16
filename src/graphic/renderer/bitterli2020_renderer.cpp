#include "bitterli2020_renderer.h"


#define RESERVOIR_RGEN_SHADER "bitterli2020_reservoir_raygen.spv"
#define RESULT_RGEN_SHADER    "bitterli2020_result_raygen.spv"


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
	bool hit;
	bool lightSource;
};


Bitterli2020::Bitterli2020(Device* device)
:Renderer(device), device(device), descriptorCollection(device),
reservoirPipeline(device), resultPipeline(device),
renderSettingsBuffers(device), rayPayloadsBuffers(device),
spatialReservoirsBuffers(device), prevTemporalReservoirs(device), nextTemporalReservoirs(device, &prevTemporalReservoirs, 1) {}

Bitterli2020::~Bitterli2020() {}

void Bitterli2020::initRenderer() {
	createBuffers();
	createDescriptorCollection();
	createPipelineLayout();
	createReservoirPipeline();
	createResultPipeline();
}

void Bitterli2020::cmdRenderFrame(size_t index, VkCommandBuffer commandBuffer) {
	descriptorCollection.cmdBind(index, commandBuffer, getPipelineLayout());

	reservoirPipeline.cmdExecutePipeline(commandBuffer);

	Renderer::cmdPipelineBarrier(reservoirPipeline.getStageMask(), resultPipeline.getStageMask(), commandBuffer);

	resultPipeline.cmdExecutePipeline(commandBuffer);
}

void Bitterli2020::updateRendererUniforms(size_t index) {
	renderSettingsBuffers.at(index).passData((void*) &renderSettings);
}

void Bitterli2020::parseRendererInput(const InputEntry& inputEntry) {
	renderSettings.visionJumpCount = inputEntry.get<u_int32_t>("visionJumpCount");
	renderSettings.candidateCount = inputEntry.get<u_int32_t>("candidateCount");
	renderSettings.sampleCount = inputEntry.get<u_int32_t>("sampleCount");
}

void Bitterli2020::createBuffers() {
	renderSettingsBuffers.bufferProperties.bufferSize = sizeof(Bitterli2020::RenderSettings);
	renderSettingsBuffers.bufferProperties.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	renderSettingsBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	renderSettingsBuffers.init();

	unsigned int bufferSize = device->renderInfo.swapchainExtend.width * device->renderInfo.swapchainExtend.height * 1.1;

	rayPayloadsBuffers.bufferProperties.bufferSize = sizeof(RayPayload) * bufferSize;
	rayPayloadsBuffers.bufferProperties.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	rayPayloadsBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	rayPayloadsBuffers.init();

	spatialReservoirsBuffers.bufferProperties.bufferSize = sizeof(Reservoir) * renderSettings.sampleCount * bufferSize;
	spatialReservoirsBuffers.bufferProperties.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	spatialReservoirsBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	spatialReservoirsBuffers.init();

	prevTemporalReservoirs.bufferProperties.bufferSize = sizeof(Reservoir) * renderSettings.sampleCount * bufferSize;
	prevTemporalReservoirs.bufferProperties.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	prevTemporalReservoirs.bufferProperties.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	prevTemporalReservoirs.init();

	Reservoir defaultReservoir = {};
	std::vector<Reservoir> reservoirs(renderSettings.sampleCount * bufferSize, defaultReservoir);

	prevTemporalReservoirs.forEach([&reservoirs](DataBuffer& buffer){
		buffer.passData((void*) reservoirs.data());
	});
}

void Bitterli2020::createDescriptorCollection() {
	descriptorCollection.bufferDescriptors.resize(6);

	descriptorCollection.bufferDescriptors.at(0) = &renderSettingsBuffers;
	descriptorCollection.bufferDescriptors.at(1) = outputImages;
	descriptorCollection.bufferDescriptors.at(2) = &rayPayloadsBuffers;
	descriptorCollection.bufferDescriptors.at(3) = &spatialReservoirsBuffers;
	descriptorCollection.bufferDescriptors.at(4) = &prevTemporalReservoirs;
	descriptorCollection.bufferDescriptors.at(5) = &nextTemporalReservoirs;

	descriptorCollection.init();

	descriptors.push_back(&descriptorCollection);
}

void Bitterli2020::createReservoirPipeline() {
	reservoirPipeline.raygenShaders.push_back(RESERVOIR_RGEN_SHADER);
	reservoirPipeline.missShaders = Renderer::RMISS_SHADERS;
	reservoirPipeline.hitShaders = Renderer::RCHIT_SHADERS;

	reservoirPipeline.pipelineLayout = getPipelineLayout();

	reservoirPipeline.width = device->renderInfo.swapchainExtend.width;
	reservoirPipeline.height = device->renderInfo.swapchainExtend.height;

	reservoirPipeline.init();
}

void Bitterli2020::createResultPipeline() {
	resultPipeline.raygenShaders.push_back(RESULT_RGEN_SHADER);
	resultPipeline.missShaders = Renderer::RMISS_SHADERS;
	resultPipeline.hitShaders = Renderer::RCHIT_SHADERS;

	resultPipeline.pipelineLayout = getPipelineLayout();

	resultPipeline.width = device->renderInfo.swapchainExtend.width;
	resultPipeline.height = device->renderInfo.swapchainExtend.height;

	resultPipeline.init();
}
