#include "majercik2019_renderer.h"


#define PROBE_RGEN_SHADER          "majercik2019_probe_raygen.spv"
#define SHADING_UPDATE_COMP_SHADER "majercik2019_shading_update_comp.spv"
#define FINAL_RGEN_SHADER          "majercik2019_final_raygen.spv"


struct Surfel {
	Vector3f rayDirection;
	Vector3f hitRadiance;
	float hitDistance;
	bool hit;
};


Majercik2019::Majercik2019(Device* device)
:Renderer(device), device(device), descriptorCollection(device),
probePipeline(device), shadingUpdatePipeline(device), finalPipeline(device),
renderSettingsBuffers(device),
surfelBuffer(device), irradianceBuffer(device), depthBuffer(device),
irradianceSampler(&irradianceBuffer), depthSampler(&depthBuffer) {}

Majercik2019::~Majercik2019() {}

void Majercik2019::initRenderer() {
	createBuffers();
	createDescriptorCollection();
	createPipelineLayout();
	createProbePipeline();
	createShadingUpdatePipeline();
	createFinalPipeline();
}

void Majercik2019::cmdRenderFrame(size_t index, VkCommandBuffer commandBuffer) {
	descriptorCollection.cmdBind(index, commandBuffer, getPipelineLayout());

	probePipeline.cmdExecutePipeline(commandBuffer);

	Renderer::cmdPipelineBarrier(probePipeline.getStageMask(), shadingUpdatePipeline.getStageMask(), commandBuffer);

	shadingUpdatePipeline.cmdExecutePipeline(commandBuffer);

	Renderer::cmdPipelineBarrier(shadingUpdatePipeline.getStageMask(), finalPipeline.getStageMask(), commandBuffer);

	finalPipeline.cmdExecutePipeline(commandBuffer);
}

void Majercik2019::updateRendererUniforms(size_t index) {
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

void Majercik2019::parseRendererInput(const InputEntry& inputEntry) {
	renderSettings.visionJumpCount = inputEntry.get<u_int32_t>("visionJumpCount");
	renderSettings.perProbeRayCount = inputEntry.get<u_int32_t>("perProbeRayCount");
	renderSettings.maxProbeRayDistance = inputEntry.get<float>("maxProbeRayDistance");
	renderSettings.probeSampleSideLength = inputEntry.get<u_int32_t>("probeSampleSideLength");
	renderSettings.depthSharpness = inputEntry.get<float>("depthSharpness");
	renderSettings.normalBias = inputEntry.get<float>("normalBias");
	renderSettings.linearBlending = inputEntry.get<u_int32_t>("linearBlending");
	renderSettings.energyPreservation = inputEntry.get<float>("energyPreservation");
	renderSettings.shadowCountProbe = inputEntry.get<u_int32_t>("shadowCountProbe");
	renderSettings.shadowCountVision = inputEntry.get<u_int32_t>("shadowCountVision");

	renderSettings.probeCount = probeData.probeCount;
	renderSettings.totalProbeCount = probeData.totalProbeCount;
	renderSettings.probeStartCorner = probeData.probeStartCorner;
	renderSettings.betweenProbeDistance = probeData.betweenProbeDistance;
}

void Majercik2019::createBuffers() {
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

	renderSettingsBuffers.bufferProperties.bufferSize = sizeof(Majercik2019::RenderSettings);
	renderSettingsBuffers.bufferProperties.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	renderSettingsBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	renderSettingsBuffers.init();

	surfelBuffer.bufferProperties.bufferSize = sizeof(Surfel) * renderSettings.totalProbeCount * renderSettings.perProbeRayCount;
	surfelBuffer.bufferProperties.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	surfelBuffer.bufferProperties.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	surfelBuffer.init();
}

void Majercik2019::createDescriptorCollection() {
	descriptorCollection.addBuffer(0, &renderSettingsBuffers);
	descriptorCollection.addBuffer(1, &surfelBuffer);
	descriptorCollection.addBuffer(2, &irradianceBuffer);
	descriptorCollection.addBuffer(3, &depthBuffer);
	descriptorCollection.addBuffer(4, &irradianceSampler);
	descriptorCollection.addBuffer(5, &depthSampler);
	descriptorCollection.addBuffer(6, outputImages);

	descriptorCollection.init();

	descriptors.push_back(&descriptorCollection);
}

void Majercik2019::createProbePipeline() {
	probePipeline.raygenShaders.push_back(PROBE_RGEN_SHADER);
	probePipeline.missShaders = Renderer::RMISS_SHADERS;
	probePipeline.hitShaders = Renderer::RCHIT_SHADERS;

	probePipeline.pipelineLayout = getPipelineLayout();

	probePipeline.width = renderSettings.totalProbeCount;
	probePipeline.height = renderSettings.perProbeRayCount;

	probePipeline.init();
}

void Majercik2019::createShadingUpdatePipeline() {
	shadingUpdatePipeline.shaderPath = SHADING_UPDATE_COMP_SHADER;

	shadingUpdatePipeline.pipelineLayout = getPipelineLayout();

	shadingUpdatePipeline.x = irradianceBuffer.bufferProperties.width;
	shadingUpdatePipeline.y = irradianceBuffer.bufferProperties.height;

	shadingUpdatePipeline.init();
}

void Majercik2019::createFinalPipeline() {
	finalPipeline.raygenShaders.push_back(FINAL_RGEN_SHADER);
	finalPipeline.missShaders = Renderer::RMISS_SHADERS;
	finalPipeline.hitShaders = Renderer::RCHIT_SHADERS;

	finalPipeline.pipelineLayout = getPipelineLayout();

	finalPipeline.width = device->renderInfo.swapchainExtend.width;
	finalPipeline.height = device->renderInfo.swapchainExtend.height;

	finalPipeline.init();
}

Vector2u Majercik2019::getIrradianceFieldSurfaceExtend() const {
	unsigned int probeExtendWBorder = renderSettings.probeSampleSideLength + 2;
	return Vector2u({
		probeExtendWBorder * renderSettings.probeCount[0] * renderSettings.probeCount[1] + 2,
		probeExtendWBorder * renderSettings.probeCount[2] + 2
	});
}
