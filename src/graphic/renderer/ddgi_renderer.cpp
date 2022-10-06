#include "ddgi_renderer.h"


#define PROBE_RGEN_SHADER          "ddgi_probe_raygen.spv"
#define SHADING_UPDATE_COMP_SHADER "ddgi_shading_update_comp.spv"
#define FINAL_RGEN_SHADER          "ddgi_final_raygen.spv"


struct Surfel {
	Vector3f rayDirection;
	Vector3f hitRadiance;
	float hitDistance;
	bool hit;
};


DDGIRenderer::DDGIRenderer(Device* device)
:Renderer(device), device(device), descriptorCollection(device),
probePipeline(device), shadingUpdatePipeline(device), finalPipeline(device),
renderSettingsBuffers(device),
surfelBuffer(device), irradianceBuffer(device), depthBuffer(device),
irradianceSampler(&irradianceBuffer), depthSampler(&depthBuffer) {}

DDGIRenderer::~DDGIRenderer() {}

void DDGIRenderer::initRenderer() {
	createBuffers();
	createDescriptorCollection();
	createPipelineLayout();
	createProbePipeline();
	createShadingUpdatePipeline();
	createFinalPipeline();
}

void DDGIRenderer::cmdRenderFrame(size_t index, VkCommandBuffer commandBuffer) {
	descriptorCollection.cmdBind(index, commandBuffer, getPipelineLayout());

	probePipeline.cmdExecutePipeline(commandBuffer);

	Renderer::cmdPipelineBarrier(probePipeline.getStageMask(), shadingUpdatePipeline.getStageMask(), commandBuffer);

	shadingUpdatePipeline.cmdExecutePipeline(commandBuffer);

	Renderer::cmdPipelineBarrier(shadingUpdatePipeline.getStageMask(), finalPipeline.getStageMask(), commandBuffer);

	finalPipeline.cmdExecutePipeline(commandBuffer);
}

void DDGIRenderer::updateRendererUniforms(size_t index) {
	renderSettingsBuffers.at(index).passData((void*) &renderSettings);

	std::vector<Surfel> surfels(renderSettings.totalProbeCount * renderSettings.perProbeRayCount);
	surfelBuffer.at(index).getData((void*) surfels.data());

	// std::ofstream file("surfels.csv");
	// file << "rayDirection;hitRadiance;hitDistance;hit" << std::endl;
	// for (const Surfel& surfel: surfels) {
	// 	file << surfel.rayDirection << ";";
	// 	file << surfel.hitRadiance << ";";
	// 	file << surfel.hitDistance << ";";
	// 	file << surfel.hit << std::endl;
	// }
	// file.close();

	// irradianceBuffer.at(index).saveImageAsNetpbm("irradiance.ppm");
	// depthBuffer.at(index).saveImageAsNetpbm("depth.ppm");
}

void DDGIRenderer::parseRendererInput(const InputEntry& inputEntry) {
	renderSettings.lightPosition = inputEntry.getVector<3, float>("lightPosition");
	renderSettings.lightJumpCount = inputEntry.get<u_int32_t>("lightJumpCount");
	renderSettings.visionJumpCount = inputEntry.get<u_int32_t>("visionJumpCount");

	renderSettings.betweenProbeDistance = inputEntry.get<float>("betweenProbeDistance");
	renderSettings.singleDirectionProbeCount = inputEntry.get<u_int32_t>("singleDirectionProbeCount");
	renderSettings.perProbeRayCount = inputEntry.get<u_int32_t>("perProbeRayCount");
	u_int32_t singleExtend = 2 * renderSettings.singleDirectionProbeCount + 1;
	renderSettings.totalProbeCount = singleExtend*singleExtend*singleExtend;

	renderSettings.maxProbeRayDistance = inputEntry.get<float>("maxProbeRayDistance");
	renderSettings.probeSampleSideLength = inputEntry.get<u_int32_t>("probeSampleSideLength");
	renderSettings.depthSharpness = inputEntry.get<float>("depthSharpness");
	renderSettings.normalBias = inputEntry.get<float>("normalBias");
	renderSettings.crushThreshold = inputEntry.get<float>("crushThreshold");
	renderSettings.linearBlending = inputEntry.get<u_int32_t>("linearBlending");
	renderSettings.energyPreservation = inputEntry.get<float>("energyPreservation");
	renderSettings.texelGetProbeDirectionFactor = inputEntry.get<float>("texelGetProbeDirectionFactor");
	renderSettings.texelGetNormalFactor = inputEntry.get<float>("texelGetNormalFactor");
}

void DDGIRenderer::createBuffers() {
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
	depthBuffer.bufferProperties.format = VK_FORMAT_R16G16B16A16_SFLOAT;
	depthBuffer.init();

	renderSettingsBuffers.bufferProperties.bufferSize = sizeof(DDGIRenderer::RenderSettings);
	renderSettingsBuffers.bufferProperties.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	renderSettingsBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	renderSettingsBuffers.init();

	surfelBuffer.bufferProperties.bufferSize = sizeof(Surfel) * renderSettings.totalProbeCount * renderSettings.perProbeRayCount;
	surfelBuffer.bufferProperties.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	surfelBuffer.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	surfelBuffer.init();
}

void DDGIRenderer::createDescriptorCollection() {
	descriptorCollection.bufferDescriptors.resize(7);

	descriptorCollection.bufferDescriptors.at(0) = &renderSettingsBuffers;
	descriptorCollection.bufferDescriptors.at(1) = &surfelBuffer;
	descriptorCollection.bufferDescriptors.at(2) = &irradianceBuffer;
	descriptorCollection.bufferDescriptors.at(3) = &depthBuffer;
	descriptorCollection.bufferDescriptors.at(4) = &irradianceSampler;
	descriptorCollection.bufferDescriptors.at(5) = &depthSampler;
	descriptorCollection.bufferDescriptors.at(6) = outputImages;

	descriptorCollection.init();

	descriptors.push_back(&descriptorCollection);
}

void DDGIRenderer::createProbePipeline() {
	probePipeline.raygenShaders.push_back(PROBE_RGEN_SHADER);
	probePipeline.missShaders.push_back(Renderer::RMISS_SHADER);
	probePipeline.hitShaders.push_back(Renderer::RCHIT_SHADER);

	probePipeline.pipelineLayout = getPipelineLayout();

	probePipeline.width = renderSettings.perProbeRayCount;
	probePipeline.height = renderSettings.totalProbeCount;

	probePipeline.init();
}

void DDGIRenderer::createShadingUpdatePipeline() {
	shadingUpdatePipeline.shaderPath = SHADING_UPDATE_COMP_SHADER;

	shadingUpdatePipeline.pipelineLayout = getPipelineLayout();

	shadingUpdatePipeline.x = irradianceBuffer.bufferProperties.width;
	shadingUpdatePipeline.y = irradianceBuffer.bufferProperties.height;

	shadingUpdatePipeline.init();
}

void DDGIRenderer::createFinalPipeline() {
	finalPipeline.raygenShaders.push_back(FINAL_RGEN_SHADER);
	finalPipeline.missShaders.push_back(Renderer::RMISS_SHADER);
	finalPipeline.hitShaders.push_back(Renderer::RCHIT_SHADER);

	finalPipeline.pipelineLayout = getPipelineLayout();

	finalPipeline.width = device->renderInfo.swapchainExtend.width;
	finalPipeline.height = device->renderInfo.swapchainExtend.height;

	finalPipeline.init();
}

Vector2u DDGIRenderer::getIrradianceFieldSurfaceExtend() const {
	unsigned int probeExtendWBorder = renderSettings.probeSampleSideLength + 2;
	unsigned int singleExtend = 2 * renderSettings.singleDirectionProbeCount + 1;
	return Vector2u({
		probeExtendWBorder * singleExtend * singleExtend + 2,
		probeExtendWBorder * singleExtend + 2
	});
}
