#include "renderer.h"


const std::string Renderer::RMISS_SHADER = "v1_miss.spv";
const std::string Renderer::RCHIT_SHADER = "v1_closesthit.spv";


Renderer::Renderer(Device* device)
:rtData(), objects(), outputImages(nullptr),
device(device), descriptorCollection(device), objDataPtrs(),
tlas(device), rtDataBuffers(device), objDataBuffers(device) {}

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
	descriptorCollection.cmdBind(index, commandBuffer, pipelineLayout);
	
	cmdRenderFrame(index, commandBuffer);
}

void Renderer::updateUniforms(size_t index) {
	rtDataBuffers.at(index).passData((void*) &rtData);

	for (size_t i = 0; i < objects.size(); ++i) {
		objects.at(i)->passBufferData(index);
		objDataBuffers.at(index).passData(objDataPtrs.at(i), i * GraphicsObject::getRTDataSize(), GraphicsObject::getRTDataSize());
	}

	updateRendererUniforms(index);
}

void Renderer::parseInput(const InputEntry& inputEntry) {
	rtData.backgroundColor = inputEntry.getVector<3, float>("backgroundColor");
	parseRendererInput(inputEntry);
}

void Renderer::passObjects(const std::vector<GraphicsObject*>& objects) {
	this->objects = objects;
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
}

void Renderer::createDescriptorCollection() {
	descriptorCollection.bindingSetIndex = 1;
	descriptorCollection.bufferDescriptors.resize(3);

	descriptorCollection.bufferDescriptors.at(0) = &tlas;
	descriptorCollection.bufferDescriptors.at(1) = &objDataBuffers;
	descriptorCollection.bufferDescriptors.at(2) = &rtDataBuffers;

	descriptorCollection.init();

	descriptors.push_back(&descriptorCollection);
}

VkPipelineLayout Renderer::getPipelineLayout() {
	return pipelineLayout;
}
