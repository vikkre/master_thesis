#include "majercik2022_bidirectional_path_tracer.h"


#define PROBE_RGEN_SHADER           "majercik2022_bidirectional_path_tracer_probe_raygen.spv"
#define PROBE_RESERVOIR_RGEN_SHADER "majercik2022_bidirectional_path_tracer_probe_reservoir_raygen.spv"
#define SHADING_UPDATE_COMP_SHADER  "majercik2022_bidirectional_path_tracer_shading_update_comp.spv"
#define RESERVOIR_RGEN_SHADER       "majercik2022_bidirectional_path_tracer_reservoir_raygen.spv"
#define FINAL_RGEN_SHADER           "majercik2022_bidirectional_path_tracer_final_raygen.spv"


struct Surfel {
	Vector3f rayDirection;
	Vector3f hitRadiance;
	float hitDistance;
	bool hit;
};

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
	Vector3f direction;
	Vector3f color;
	u_int32_t hit;
	u_int32_t lightSource;
};


Majercik2022BidirectionalPathTracer::Majercik2022BidirectionalPathTracer(Device* device)
:Renderer(device), device(device), descriptorCollection(device),
probeReservoirPipeline(device), probePipeline(device), shadingUpdatePipeline(device),
reservoirPipeline(device), finalPipeline(device),
renderSettingsBuffers(device),
surfelBuffer(device), irradianceBuffer(device), depthBuffer(device),
irradianceSampler(&irradianceBuffer), depthSampler(&depthBuffer),
rayPayloadsBuffers(device), spatialReservoirsBuffers(device),
probeRayPayloadsBuffers(device), probeSpatialReservoirsBuffers(device) {}

Majercik2022BidirectionalPathTracer::~Majercik2022BidirectionalPathTracer() {}

void Majercik2022BidirectionalPathTracer::initRenderer() {
	createBuffers();
	createDescriptorCollection();
	createPipelineLayout();
	createProbeReservoirPipeline();
	createProbePipeline();
	createShadingUpdatePipeline();
	createReservoirPipeline();
	createFinalPipeline();
}

void Majercik2022BidirectionalPathTracer::cmdRenderFrame(size_t index, VkCommandBuffer commandBuffer) {
	descriptorCollection.cmdBind(index, commandBuffer, getPipelineLayout());

	probeReservoirPipeline.cmdExecutePipeline(commandBuffer);

	Renderer::cmdPipelineBarrier(probeReservoirPipeline.getStageMask(), probePipeline.getStageMask(), commandBuffer);

	probePipeline.cmdExecutePipeline(commandBuffer);

	Renderer::cmdPipelineBarrier(probePipeline.getStageMask(), shadingUpdatePipeline.getStageMask(), commandBuffer);

	shadingUpdatePipeline.cmdExecutePipeline(commandBuffer);

	Renderer::cmdPipelineBarrier(shadingUpdatePipeline.getStageMask(), reservoirPipeline.getStageMask(), commandBuffer);

	reservoirPipeline.cmdExecutePipeline(commandBuffer);

	Renderer::cmdPipelineBarrier(reservoirPipeline.getStageMask(), finalPipeline.getStageMask(), commandBuffer);

	finalPipeline.cmdExecutePipeline(commandBuffer);
}

void Majercik2022BidirectionalPathTracer::updateRendererUniforms(size_t index) {
	renderSettingsBuffers.at(index).passData((void*) &renderSettings);

	static unsigned int num = 0;
	if (num > 10) return;

	// std::vector<Surfel> surfels(renderSettings.totalProbeCount * renderSettings.perProbeRayCount);
	// surfelBuffer.at(index).getData((void*) surfels.data());

	// std::ofstream file("surfels.csv");
	// std::ofstream file(std::string("surfels.csv") + std::to_string(num));
	// file << "rayDirection;hitRadiance;hitDistance;hit" << std::endl;
	// for (const Surfel& surfel: surfels) {
	// 	file << surfel.rayDirection << ";";
	// 	file << surfel.hitRadiance << ";";
	// 	file << surfel.hitDistance << ";";
	// 	file << surfel.hit << std::endl;
	// }
	// file.close();

	// irradianceBuffer.at(index).saveImageAsNetpbm(std::string("irradiance.ppm") + std::to_string(num));
	// depthBuffer.at(index).saveImageAsNetpbm("depth.ppm");

	num++;
}

void Majercik2022BidirectionalPathTracer::parseRendererInput(const InputEntry& inputEntry) {
	renderSettings.visionJumpCount = inputEntry.get<u_int32_t>("visionJumpCount");
	renderSettings.lightJumpCount  = inputEntry.get<u_int32_t>("lightJumpCount");
	renderSettings.maxDepth        = inputEntry.get<u_int32_t>("maxDepth");
	
	renderSettings.hysteresis            = inputEntry.get<float>("hysteresis");
	renderSettings.perProbeRayCount      = inputEntry.get<u_int32_t>("perProbeRayCount");
	renderSettings.maxProbeRayDistance   = inputEntry.get<float>("maxProbeRayDistance");
	renderSettings.probeSampleSideLength = inputEntry.get<u_int32_t>("probeSampleSideLength");
	renderSettings.depthSharpness        = inputEntry.get<float>("depthSharpness");
	renderSettings.normalBias            = inputEntry.get<float>("normalBias");
	renderSettings.linearBlending        = inputEntry.get<u_int32_t>("linearBlending");
	renderSettings.energyPreservation    = inputEntry.get<float>("energyPreservation");

	renderSettings.candidateCount = inputEntry.get<u_int32_t>("candidateCount");
	renderSettings.sampleCount    = inputEntry.get<u_int32_t>("sampleCount");

	renderSettings.probeCandidateCount  = inputEntry.get<u_int32_t>("probeCandidateCount");
	renderSettings.probeSampleCount     = inputEntry.get<u_int32_t>("probeSampleCount");
	renderSettings.probeVisionJumpCount = inputEntry.get<u_int32_t>("probeVisionJumpCount");
	renderSettings.probeLightJumpCount  = inputEntry.get<u_int32_t>("probeLightJumpCount");
	renderSettings.probeMaxDepth        = inputEntry.get<u_int32_t>("probeMaxDepth");

	renderSettings.probeCount           = probeData.probeCount;
	renderSettings.totalProbeCount      = probeData.totalProbeCount;
	renderSettings.probeStartCorner     = probeData.probeStartCorner;
	renderSettings.betweenProbeDistance = probeData.betweenProbeDistance;
}

void Majercik2022BidirectionalPathTracer::createBuffers() {
	Vector2u extend = getIrradianceFieldSurfaceExtend();
	ImageBuffer::Properties shadingBufferProperties;
	shadingBufferProperties.width = extend[0];
	shadingBufferProperties.height = extend[1];
	shadingBufferProperties.tiling = VK_IMAGE_TILING_OPTIMAL;
	shadingBufferProperties.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	shadingBufferProperties.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	shadingBufferProperties.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
	shadingBufferProperties.layout = VK_IMAGE_LAYOUT_GENERAL;
	shadingBufferProperties.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	shadingBufferProperties.createImageView = true;
	shadingBufferProperties.createSampler = true;

	irradianceBuffer.bufferProperties = shadingBufferProperties;
	irradianceBuffer.bufferProperties.format = VK_FORMAT_B8G8R8A8_UNORM;
	irradianceBuffer.init();

	depthBuffer.bufferProperties = shadingBufferProperties;
	depthBuffer.bufferProperties.format = VK_FORMAT_R16G16_SFLOAT;
	depthBuffer.init();

	renderSettingsBuffers.bufferProperties.bufferSize = sizeof(Majercik2022BidirectionalPathTracer::RenderSettings);
	renderSettingsBuffers.bufferProperties.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	renderSettingsBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	renderSettingsBuffers.init();

	surfelBuffer.bufferProperties.bufferSize = sizeof(Surfel) * renderSettings.totalProbeCount * renderSettings.perProbeRayCount;
	surfelBuffer.bufferProperties.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	surfelBuffer.bufferProperties.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	surfelBuffer.init();

	unsigned int bufferSize = device->renderInfo.swapchainExtend.width * device->renderInfo.swapchainExtend.height;

	rayPayloadsBuffers.bufferProperties.bufferSize = sizeof(RayPayload) * bufferSize;
	rayPayloadsBuffers.bufferProperties.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	rayPayloadsBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	rayPayloadsBuffers.init();

	spatialReservoirsBuffers.bufferProperties.bufferSize = sizeof(Reservoir) * renderSettings.sampleCount * bufferSize;
	spatialReservoirsBuffers.bufferProperties.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	spatialReservoirsBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	spatialReservoirsBuffers.init();

	unsigned int probeBufferSize = renderSettings.totalProbeCount * renderSettings.perProbeRayCount;

	probeRayPayloadsBuffers.bufferProperties.bufferSize = sizeof(RayPayload) * probeBufferSize;
	probeRayPayloadsBuffers.bufferProperties.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	probeRayPayloadsBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	probeRayPayloadsBuffers.init();

	probeSpatialReservoirsBuffers.bufferProperties.bufferSize = sizeof(Reservoir) * renderSettings.probeSampleCount * probeBufferSize;
	probeSpatialReservoirsBuffers.bufferProperties.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	probeSpatialReservoirsBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	probeSpatialReservoirsBuffers.init();
}

void Majercik2022BidirectionalPathTracer::createDescriptorCollection() {
	descriptorCollection.addBuffer(0, &renderSettingsBuffers);
	descriptorCollection.addBuffer(1, &surfelBuffer);
	descriptorCollection.addBuffer(2, &irradianceBuffer);
	descriptorCollection.addBuffer(3, &depthBuffer);
	descriptorCollection.addBuffer(4, &irradianceSampler);
	descriptorCollection.addBuffer(5, &depthSampler);
	descriptorCollection.addBuffer(6, outputImages);
	descriptorCollection.addBuffer(7, &rayPayloadsBuffers);
	descriptorCollection.addBuffer(8, &spatialReservoirsBuffers);
	descriptorCollection.addBuffer(9, &probeRayPayloadsBuffers);
	descriptorCollection.addBuffer(10, &probeSpatialReservoirsBuffers);

	descriptorCollection.init();

	descriptors.push_back(&descriptorCollection);
}

void Majercik2022BidirectionalPathTracer::createProbeReservoirPipeline() {
	probeReservoirPipeline.raygenShaders.push_back(PROBE_RESERVOIR_RGEN_SHADER);
	probeReservoirPipeline.missShaders = Renderer::RMISS_SHADERS;
	probeReservoirPipeline.hitShaders = Renderer::RCHIT_SHADERS;

	probeReservoirPipeline.pipelineLayout = getPipelineLayout();

	probeReservoirPipeline.width = renderSettings.totalProbeCount;
	probeReservoirPipeline.height = renderSettings.perProbeRayCount;

	probeReservoirPipeline.init();
}

void Majercik2022BidirectionalPathTracer::createProbePipeline() {
	probePipeline.raygenShaders.push_back(PROBE_RGEN_SHADER);
	probePipeline.missShaders = Renderer::RMISS_SHADERS;
	probePipeline.hitShaders = Renderer::RCHIT_SHADERS;

	probePipeline.pipelineLayout = getPipelineLayout();

	probePipeline.width = renderSettings.totalProbeCount;
	probePipeline.height = renderSettings.perProbeRayCount;

	probePipeline.init();
}

void Majercik2022BidirectionalPathTracer::createShadingUpdatePipeline() {
	shadingUpdatePipeline.shaderPath = SHADING_UPDATE_COMP_SHADER;

	shadingUpdatePipeline.pipelineLayout = getPipelineLayout();

	shadingUpdatePipeline.x = irradianceBuffer.bufferProperties.width;
	shadingUpdatePipeline.y = irradianceBuffer.bufferProperties.height;

	shadingUpdatePipeline.init();
}

void Majercik2022BidirectionalPathTracer::createReservoirPipeline() {
	reservoirPipeline.raygenShaders.push_back(RESERVOIR_RGEN_SHADER);
	reservoirPipeline.missShaders = Renderer::RMISS_SHADERS;
	reservoirPipeline.hitShaders = Renderer::RCHIT_SHADERS;

	reservoirPipeline.pipelineLayout = getPipelineLayout();

	reservoirPipeline.width = device->renderInfo.swapchainExtend.width;
	reservoirPipeline.height = device->renderInfo.swapchainExtend.height;

	reservoirPipeline.init();
}

void Majercik2022BidirectionalPathTracer::createFinalPipeline() {
	finalPipeline.raygenShaders.push_back(FINAL_RGEN_SHADER);
	finalPipeline.missShaders = Renderer::RMISS_SHADERS;
	finalPipeline.hitShaders = Renderer::RCHIT_SHADERS;

	finalPipeline.pipelineLayout = getPipelineLayout();

	finalPipeline.width = device->renderInfo.swapchainExtend.width;
	finalPipeline.height = device->renderInfo.swapchainExtend.height;

	finalPipeline.init();
}

Vector2u Majercik2022BidirectionalPathTracer::getIrradianceFieldSurfaceExtend() const {
	unsigned int probeExtendWBorder = renderSettings.probeSampleSideLength + 2;
	return Vector2u({
		probeExtendWBorder * renderSettings.probeCount[0] * renderSettings.probeCount[1] + 2,
		probeExtendWBorder * renderSettings.probeCount[2] + 2
	});
}
