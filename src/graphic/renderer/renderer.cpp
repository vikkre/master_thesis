#include "renderer.h"


const std::vector<std::string> Renderer::RMISS_SHADERS = {"v1_miss.spv", "v1_shadow.spv"};
const std::vector<std::string> Renderer::RCHIT_SHADERS = {"v1_closesthit.spv"};


Renderer::Renderer(Device* device)
:rtData(), objects(), outputImages(nullptr),
device(device), descriptorCollection(device), objDataPtrs(),
tlas(device), rtDataBuffers(device),
objDataBuffers(device), lightSourceDataBuffers(device) {}

Renderer::~Renderer() {
	vkDestroyPipelineLayout(device->getDevice(), pipelineLayout, nullptr);
}

void Renderer::init() {
	createTLAS();
	createBuffers();
	createDescriptorCollection();
	initRenderer();
}

void Renderer::cmdRender(size_t index, VkCommandBuffer commandBuffer) {
	tlas.at(index).cmdUpdate(commandBuffer);

	cmdPipelineBarrier(VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, commandBuffer);

	descriptorCollection.cmdBind(index, commandBuffer, pipelineLayout);
	
	cmdRenderFrame(index, commandBuffer);
}

void Renderer::updateUniforms(size_t index) {
	rtData.lightSourceCount = lightSources.size();
	rtDataBuffers.at(index).passData((void*) &rtData);

	size_t currentLightSource = 0;
	for (size_t i = 0; i < objects.size(); ++i) {
		GraphicsObject::ObjectInfo info = objects.at(i)->getObjectInfo();
		tlas.bufferProperties.blasInstances.at(i) = info.instance;
		objDataPtrs.at(i) = info.dataPtr;

		objects.at(i)->passBufferData(index);
		objDataBuffers.at(index).passData(objDataPtrs.at(i), i * GraphicsObject::getRTDataSize(), GraphicsObject::getRTDataSize());

		if (objects.at(i)->rtData.lightSource == 1) {
			lightSourceDataBuffers.at(index).passData(objDataPtrs.at(i), currentLightSource * GraphicsObject::getRTDataSize(), GraphicsObject::getRTDataSize());
			++currentLightSource;
		}
	}

	tlas.at(index).properties = tlas.bufferProperties;
	tlas.at(index).updateUniforms();

	updateRendererUniforms(index);
}

void Renderer::parseInput(const InputEntry& inputEntry) {
	parseRendererInput(inputEntry);
}

void Renderer::passObjects(const std::vector<GraphicsObject*>& objects) {
	this->objects = objects;
}

void Renderer::passLightSources(const std::vector<GraphicsObject*>& lightSources) {
	this->lightSources = lightSources;
}

void Renderer::setOutputImageBuffer(MultiBufferDescriptor<ImageBuffer>* outputImageBuffer) {
	outputImages = outputImageBuffer;
}

void Renderer::cmdPipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkCommandBuffer commandBuffer) {
	vkCmdPipelineBarrier(
		commandBuffer,
		srcStageMask, dstStageMask,
		0,
		0, nullptr,
		0, nullptr,
		0, nullptr
	);
}

void Renderer::createPipelineLayout() {
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts(descriptors.size());

	for (const DescriptorCollection* descriptor: descriptors) {
		descriptorSetLayouts.at(descriptor->bindingSetIndex) = descriptor->getLayout();
	}

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size();
	pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();

	if (vkCreatePipelineLayout(device->getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw InitException("vkCreatePipelineLayout", "failed to create pipeline layout!");
	}
}

void Renderer::createTLAS() {
	for (GraphicsObject* obj: objects) {
		GraphicsObject::ObjectInfo info = obj->getObjectInfo();
		tlas.bufferProperties.blasInstances.push_back(info.instance);
		objDataPtrs.push_back(info.dataPtr);
	}

	tlas.init();
}


void Renderer::createBuffers() {
	rtDataBuffers.bufferProperties.bufferSize = sizeof(Renderer::RayTracingData);
	rtDataBuffers.bufferProperties.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	rtDataBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	rtDataBuffers.init();

	objDataBuffers.bufferProperties.bufferSize = GraphicsObject::getRTDataSize() * objDataPtrs.size();
	objDataBuffers.bufferProperties.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	objDataBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	objDataBuffers.init();

	lightSourceDataBuffers.bufferProperties.bufferSize = GraphicsObject::getRTDataSize() * lightSources.size();
	lightSourceDataBuffers.bufferProperties.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	lightSourceDataBuffers.bufferProperties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	lightSourceDataBuffers.init();
}

void Renderer::createDescriptorCollection() {
	descriptorCollection.bindingSetIndex = 1;
	descriptorCollection.bufferDescriptors.resize(4);

	descriptorCollection.bufferDescriptors.at(0) = &tlas;
	descriptorCollection.bufferDescriptors.at(1) = &objDataBuffers;
	descriptorCollection.bufferDescriptors.at(2) = &rtDataBuffers;
	descriptorCollection.bufferDescriptors.at(3) = &lightSourceDataBuffers;

	descriptorCollection.init();

	descriptors.push_back(&descriptorCollection);
}

VkPipelineLayout Renderer::getPipelineLayout() {
	return pipelineLayout;
}
