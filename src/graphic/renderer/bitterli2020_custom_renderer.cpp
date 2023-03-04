#include "bitterli2020_custom_renderer.h"


#define RESERVOIR_RGEN_SHADER "bitterli2020_custom_reservoir_raygen.spv"
#define RESULT_RGEN_SHADER    "bitterli2020_custom_result_raygen.spv"


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


Bitterli2020Custom::Bitterli2020Custom(Device* device)
:Renderer(device), device(device), descriptorCollection(device),
reservoirPipeline(device), resultPipeline(device),
renderSettingsBuffers(device), probeReservoirs(device) {}

Bitterli2020Custom::~Bitterli2020Custom() {}

void Bitterli2020Custom::initRenderer() {
	createBuffers();
	createDescriptorCollection();
	createPipelineLayout();
	createReservoirPipeline();
	createResultPipeline();
}

void Bitterli2020Custom::cmdRenderFrame(size_t index, VkCommandBuffer commandBuffer) {
	descriptorCollection.cmdBind(index, commandBuffer, getPipelineLayout());

	reservoirPipeline.cmdExecutePipeline(commandBuffer);

	Renderer::cmdPipelineBarrier(reservoirPipeline.getStageMask(), resultPipeline.getStageMask(), commandBuffer);

	resultPipeline.cmdExecutePipeline(commandBuffer);
}

void Bitterli2020Custom::updateRendererUniforms(size_t index) {
	renderSettingsBuffers.at(index).passData((void*) &renderSettings);
}

void Bitterli2020Custom::parseRendererInput(const InputEntry& inputEntry) {
	renderSettings.visionJumpCount = inputEntry.get<u_int32_t>("visionJumpCount");
	renderSettings.candidateCount = inputEntry.get<u_int32_t>("candidateCount");
	renderSettings.sampleCount = inputEntry.get<u_int32_t>("sampleCount");
	
	renderSettings.probeCount = probeData.probeCount;
	renderSettings.totalProbeCount = probeData.totalProbeCount;
	renderSettings.probeStartCorner = probeData.probeStartCorner;
	renderSettings.betweenProbeDistance = probeData.betweenProbeDistance;
}

void Bitterli2020Custom::createBuffers() {
	renderSettingsBuffers.bufferProperties.bufferSize = sizeof(Bitterli2020Custom::RenderSettings);
	renderSettingsBuffers.bufferProperties.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	renderSettingsBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	renderSettingsBuffers.init();

	unsigned int probeDataCount = renderSettings.totalProbeCount * renderSettings.sampleCount;
	probeReservoirs.bufferProperties.bufferSize = sizeof(Reservoir) * probeDataCount;
	probeReservoirs.bufferProperties.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	probeReservoirs.bufferProperties.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	probeReservoirs.init();

	Reservoir emptryProbeReservoir = {};
	std::vector<Reservoir> emptryProbeReservoirs(probeDataCount, emptryProbeReservoir);

	probeReservoirs.forEach([&emptryProbeReservoirs](DataBuffer& buffer){
		buffer.passData((void*) emptryProbeReservoirs.data());
	});
}

void Bitterli2020Custom::createDescriptorCollection() {
	descriptorCollection.addBuffer(0, &renderSettingsBuffers);
	descriptorCollection.addBuffer(1, outputImages);
	descriptorCollection.addBuffer(2, &probeReservoirs);

	descriptorCollection.init();

	descriptors.push_back(&descriptorCollection);
}

void Bitterli2020Custom::createReservoirPipeline() {
	reservoirPipeline.raygenShaders.push_back(RESERVOIR_RGEN_SHADER);
	reservoirPipeline.missShaders = Renderer::RMISS_SHADERS;
	reservoirPipeline.hitShaders = Renderer::RCHIT_SHADERS;

	reservoirPipeline.pipelineLayout = getPipelineLayout();

	reservoirPipeline.width = renderSettings.totalProbeCount;
	reservoirPipeline.height = renderSettings.sampleCount;

	reservoirPipeline.init();
}

void Bitterli2020Custom::createResultPipeline() {
	resultPipeline.raygenShaders.push_back(RESULT_RGEN_SHADER);
	resultPipeline.missShaders = Renderer::RMISS_SHADERS;
	resultPipeline.hitShaders = Renderer::RCHIT_SHADERS;

	resultPipeline.pipelineLayout = getPipelineLayout();

	resultPipeline.width = device->renderInfo.swapchainExtend.width;
	resultPipeline.height = device->renderInfo.swapchainExtend.height;

	resultPipeline.init();
}
