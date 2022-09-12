#include "ddgi_renderer.h"


#define RCHIT_SHADER               "ddgi_closesthit.spv"
#define RMISS_SHADER               "ddgi_miss.spv"
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
:device(device), descriptorCollection(device),
probePipeline(device), shadingUpdatePipeline(device), finalPipeline(device),
objDataPtrs(),
tlas(device), objDataBuffers(device), globalDataBuffers(device), renderSettingsBuffers(device),
surfelBuffer(device), irradianceBuffer(device), depthBuffer(device) {}

DDGIRenderer::~DDGIRenderer() {}

void DDGIRenderer::init() {
	createTLAS();
	createBuffers();
	createDescriptorCollection();
	createProbePipeline();
	createShadingUpdatePipeline();
	createFinalPipeline();
}

void DDGIRenderer::cmdRender(size_t index, VkCommandBuffer commandBuffer) {
	descriptorCollection.cmdBind(index, commandBuffer);

	probePipeline.cmdExecutePipeline(commandBuffer);

	Renderer::cmdPipelineBarrier(probePipeline.getStageMask(), shadingUpdatePipeline.getStageMask(), commandBuffer);

	shadingUpdatePipeline.cmdExecutePipeline(commandBuffer);

	Renderer::cmdPipelineBarrier(shadingUpdatePipeline.getStageMask(), finalPipeline.getStageMask(), commandBuffer);

	finalPipeline.cmdExecutePipeline(commandBuffer);
}

void DDGIRenderer::updateUniforms(size_t index) {
	globalDataBuffers.at(index).passData((void*) &globalData);
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

	irradianceBuffer.at(index).saveImageAsNetpbm("irradiance.ppm");
	// depthBuffer.at(index).saveImageAsNetpbm("depth.ppm");

	for (size_t i = 0; i < objects.size(); ++i) {
		objects.at(i)->passBufferData(index);
		objDataBuffers.at(index).passData(objDataPtrs.at(i), i * GraphicsObject::getRTDataSize(), GraphicsObject::getRTDataSize());
	}
}

void DDGIRenderer::parseInput(const InputEntry& inputEntry) {
	renderSettings.backgroundColor = inputEntry.getVector<3, float>("backgroundColor");
	renderSettings.lightPosition = inputEntry.getVector<3, float>("lightPosition");

	renderSettings.betweenProbeDistance = inputEntry.get<float>("betweenProbeDistance");
	renderSettings.singleDirectionProbeCount = inputEntry.get<u_int32_t>("singleDirectionProbeCount");
	renderSettings.perProbeRayCount = inputEntry.get<u_int32_t>("perProbeRayCount");
	u_int32_t singleExtend = 2 * renderSettings.singleDirectionProbeCount + 1;
	renderSettings.totalProbeCount = singleExtend*singleExtend*singleExtend;

	renderSettings.maxProbeRayDistance = inputEntry.get<float>("maxProbeRayDistance");
	renderSettings.probeSampleSideLength = inputEntry.get<u_int32_t>("probeSampleSideLength");
	renderSettings.depthSharpness = inputEntry.get<float>("depthSharpness");
}

void DDGIRenderer::createTLAS() {
	for (GraphicsObject* obj: objects) {
		GraphicsObject::ObjectInfo info = obj->getObjectInfo();
		tlas.bufferProperties.blasInstances.push_back(info.instance);
		objDataPtrs.push_back(info.dataPtr);
	}

	tlas.init();
}

void DDGIRenderer::createBuffers() {
	Vector2u extend = getIrradianceFieldSurfaceExtend();
	ImageBuffer::Properties shadingBufferProperties;
	shadingBufferProperties.width = extend[0];
	shadingBufferProperties.height = extend[1];
	shadingBufferProperties.tiling = VK_IMAGE_TILING_OPTIMAL;
	shadingBufferProperties.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
	shadingBufferProperties.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	shadingBufferProperties.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
	shadingBufferProperties.layout = VK_IMAGE_LAYOUT_GENERAL;
	shadingBufferProperties.createImageView = true;
	shadingBufferProperties.createSampler = true;
	// shadingBufferProperties.format = VK_FORMAT_B8G8R8A8_UNORM;

	irradianceBuffer.bufferProperties = shadingBufferProperties;
	irradianceBuffer.bufferProperties.format = VK_FORMAT_B8G8R8A8_UNORM;
	irradianceBuffer.init();

	depthBuffer.bufferProperties = shadingBufferProperties;
	depthBuffer.bufferProperties.format = VK_FORMAT_R16G16B16A16_UNORM;
	depthBuffer.init();

	objDataBuffers.bufferProperties.bufferSize = GraphicsObject::getRTDataSize() * objDataPtrs.size();
	objDataBuffers.bufferProperties.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	objDataBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	objDataBuffers.init();

	globalDataBuffers.bufferProperties.bufferSize = sizeof(DDGIRenderer::GlobalData);
	globalDataBuffers.bufferProperties.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	globalDataBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	globalDataBuffers.init();

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
	descriptorCollection.bufferDescriptors.resize(8);

	descriptorCollection.bufferDescriptors.at(0) = &tlas;
	descriptorCollection.bufferDescriptors.at(1) = &objDataBuffers;
	descriptorCollection.bufferDescriptors.at(2) = &globalDataBuffers;
	descriptorCollection.bufferDescriptors.at(3) = &renderSettingsBuffers;
	descriptorCollection.bufferDescriptors.at(4) = &surfelBuffer;
	descriptorCollection.bufferDescriptors.at(5) = &irradianceBuffer;
	descriptorCollection.bufferDescriptors.at(6) = &depthBuffer;
	descriptorCollection.bufferDescriptors.at(7) = outputImages;

	descriptorCollection.init();
}

void DDGIRenderer::createProbePipeline() {
	probePipeline.raygenShaders.push_back(PROBE_RGEN_SHADER);
	probePipeline.missShaders.push_back(RMISS_SHADER);
	probePipeline.hitShaders.push_back(RCHIT_SHADER);

	probePipeline.pipelineLayout = descriptorCollection.getPipelineLayout();

	probePipeline.width = renderSettings.perProbeRayCount;
	probePipeline.height = renderSettings.totalProbeCount;

	probePipeline.init();
}

void DDGIRenderer::createShadingUpdatePipeline() {
	shadingUpdatePipeline.shaderPath = SHADING_UPDATE_COMP_SHADER;

	shadingUpdatePipeline.pipelineLayout = descriptorCollection.getPipelineLayout();

	shadingUpdatePipeline.x = irradianceBuffer.bufferProperties.width;
	shadingUpdatePipeline.y = irradianceBuffer.bufferProperties.height;

	shadingUpdatePipeline.init();
}

void DDGIRenderer::createFinalPipeline() {
	finalPipeline.raygenShaders.push_back(FINAL_RGEN_SHADER);
	finalPipeline.missShaders.push_back(RMISS_SHADER);
	finalPipeline.hitShaders.push_back(RCHIT_SHADER);

	finalPipeline.pipelineLayout = descriptorCollection.getPipelineLayout();

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
