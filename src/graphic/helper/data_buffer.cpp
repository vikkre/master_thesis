#include "data_buffer.h"

#include "../device.h"
#include "function_load.h"
#include "command_buffer.h"


DataBuffer::DataBuffer(const Device* device)
:Buffer(), bufferSize(0), usage(), properties(0),
device(device),
useTransferBuffer(false), buffer(VK_NULL_HANDLE), memory(VK_NULL_HANDLE) {}

DataBuffer::~DataBuffer() {
	vkDeviceWaitIdle(device->getDevice());
	
	vkDestroyBuffer(device->getDevice(), buffer, nullptr);
	vkFreeMemory(device->getDevice(), memory, nullptr);
}

void DataBuffer::init() {
	useTransferBuffer = (properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0;

	createBuffer(usage, properties, buffer, memory);

	bufferInfo = {};
	bufferInfo.buffer = buffer;
	bufferInfo.offset = 0;
	bufferInfo.range = bufferSize;
}

void DataBuffer::passData(void* data) {
	passData(data, 0, (size_t) bufferSize);
}

void DataBuffer::passData(void* data, size_t start, size_t length) {
	if (useTransferBuffer) {
		if ((usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT) == 0) {
			throw InitException("DataBuffer::passData", "Cannot pass data to buffer! Missing 'VK_BUFFER_USAGE_TRANSFER_DST_BIT'?");
		}

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory
		);

		void* dst;
		vkMapMemory(device->getDevice(), stagingBufferMemory, start, length, 0, &dst);
		memcpy(dst, data, length);
		vkUnmapMemory(device->getDevice(), stagingBufferMemory);
		dst = nullptr;

		SingleUseCommandBuffer commandBuffer(device, &device->getQueues().getTransferQueue());
		commandBuffer.start();

		VkBufferCopy copyRegion{};
		copyRegion.size = length;
		vkCmdCopyBuffer(commandBuffer.getCommandBuffer(), stagingBuffer, buffer, 1, &copyRegion);

		commandBuffer.end();

		vkDestroyBuffer(device->getDevice(), stagingBuffer, nullptr);
		vkFreeMemory(device->getDevice(), stagingBufferMemory, nullptr);

	} else {
		void* dst;
		vkMapMemory(device->getDevice(), memory, start, length, 0, &dst);
		memcpy(dst, data, length);
		vkUnmapMemory(device->getDevice(), memory);
		dst = nullptr;
	}
}

void DataBuffer::getData(void* data) {
	getData(data, 0, (size_t) bufferSize);
}

void DataBuffer::getData(void* data, size_t start, size_t length) {
	if (useTransferBuffer) {
		if ((usage & VK_BUFFER_USAGE_TRANSFER_SRC_BIT) == 0) {
			throw InitException("DataBuffer::getData", "Cannot get data from buffer! Missing 'VK_BUFFER_USAGE_TRANSFER_SRC_BIT'?");
		}

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(
			VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory
		);

		SingleUseCommandBuffer commandBuffer(device, &device->getQueues().getTransferQueue());
		commandBuffer.start();

		VkBufferCopy copyRegion{};
		copyRegion.size = length;
		vkCmdCopyBuffer(commandBuffer.getCommandBuffer(), buffer, stagingBuffer, 1, &copyRegion);

		commandBuffer.end();

		void* dst;
		vkMapMemory(device->getDevice(), stagingBufferMemory, start, length, 0, &dst);
		memcpy(data, dst, length);
		vkUnmapMemory(device->getDevice(), stagingBufferMemory);
		dst = nullptr;

		vkDestroyBuffer(device->getDevice(), stagingBuffer, nullptr);
		vkFreeMemory(device->getDevice(), stagingBufferMemory, nullptr);

	} else {
		void* dst;
		vkMapMemory(device->getDevice(), memory, start, length, 0, &dst);
		memcpy(data, dst, length);
		vkUnmapMemory(device->getDevice(), memory);
		dst = nullptr;
	}
}

VkWriteDescriptorSet DataBuffer::getWriteDescriptorSet(VkDescriptorSet descriptorSet, uint32_t binding) const {
	VkWriteDescriptorSet writeSet{};

	writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeSet.pNext = nullptr;
	writeSet.dstSet = descriptorSet;
	writeSet.dstBinding = binding;
	writeSet.descriptorCount = 1;
	writeSet.descriptorType = getDescriptorType();
	writeSet.pBufferInfo = &bufferInfo;

	return writeSet;
}

VkDescriptorType DataBuffer::getDescriptorType() const {
	VkDescriptorType type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	if ((usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) != 0) {
		type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	}
	return type;
}

void DataBuffer::createBuffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& dstBuffer, VkDeviceMemory& dstMemory) {
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = bufferSize;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device->getDevice(), &bufferInfo, nullptr, &dstBuffer) != VK_SUCCESS) {
		throw InitException("vkCreateBuffer", "could not create buffer");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device->getDevice(), dstBuffer, &memRequirements);

	VkMemoryAllocateFlagsInfo allocFlags{};
	allocFlags.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
	allocFlags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = &allocFlags;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = device->findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(device->getDevice(), &allocInfo, nullptr, &dstMemory) != VK_SUCCESS) {
		throw InitException("vkAllocateMemory", "");
	}

	vkBindBufferMemory(device->getDevice(), dstBuffer, dstMemory, 0);
}

const VkBuffer& DataBuffer::getBuffer() const {
	return buffer;
}

VkDeviceAddress DataBuffer::getAddress() const {
	VkBufferDeviceAddressInfo info{};
	info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	info.buffer = buffer;
	return vkGetBufferDeviceAddress(device->getDevice(), &info);
}
