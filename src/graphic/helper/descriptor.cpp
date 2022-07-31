#include "descriptor.h"

#include "../device.h"


Descriptor::Descriptor(const Device* device)
:descriptorPool(VK_NULL_HANDLE), bufferSize(0), bindingSetIndex(0),
dstBinding(0), descriptorType(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER), setLayout(VK_NULL_HANDLE),
buffer(nullptr), device(device), descriptorSet(VK_NULL_HANDLE) {}

Descriptor::~Descriptor() {
	delete buffer;
}

void Descriptor::init() {
	createBuffer();
	createDescriptorSet();
}

void Descriptor::bind(const VkCommandBuffer* commandBuffer) {
	vkCmdBindDescriptorSets(
		*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		device->renderInfo.renderPipeline->getPipelineLayout(),
		bindingSetIndex, 1, &descriptorSet,
		0, nullptr
	);
}

void Descriptor::createBuffer() {
	if (buffer == nullptr) {
		buffer = new DataBuffer(device);

		buffer->bufferSize = bufferSize;
		buffer->usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		buffer->properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		buffer->init();
	}
}

void Descriptor::createDescriptorSet() {
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &setLayout;

	if (vkAllocateDescriptorSets(device->getDevice(), &allocInfo, &descriptorSet) != VK_SUCCESS) {
		throw InitException("vkAllocateDescriptorSets", "failed to allocate descriptor sets!");
	}

	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.offset = 0;
	bufferInfo.buffer = buffer->getBuffer();
	bufferInfo.range = buffer->bufferSize;

	VkWriteDescriptorSet write;
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.pNext = nullptr;
	write.dstSet = descriptorSet;
	write.dstBinding = dstBinding;
	write.dstArrayElement = 0;
	write.descriptorType = descriptorType;
	write.descriptorCount = 1;
	write.pBufferInfo = &bufferInfo;

	vkUpdateDescriptorSets(device->getDevice(), 1, &write, 0, nullptr);
}

DataBuffer& Descriptor::getBuffer() {
	return *buffer;
}
