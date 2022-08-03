#pragma once

#include <vulkan/vulkan.h>

#include "buffer.h"
#include "../../init_exception.h"


class Device;

class DataBuffer: public Buffer {
	public:
		DataBuffer(const Device* device);
		~DataBuffer();

		void init();
		void passData(void* data);
		void passData(void* data, size_t start, size_t length);
		void getData(void* data);
		void getData(void* data, size_t start, size_t length);

		const VkBuffer& getBuffer() const;
		VkDeviceAddress getAddress() const;
		virtual VkWriteDescriptorSet getWriteDescriptorSet(VkDescriptorSet descriptorSet, uint32_t binding) const override;
		virtual VkDescriptorType getDescriptorType() const override;

		struct Properties {
			VkDeviceSize bufferSize;
			VkBufferUsageFlags usage;
			VkMemoryPropertyFlags properties;
		} properties;

	private:
		void createBuffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& dstBuffer, VkDeviceMemory& dstMemory);

		const Device* device;

		bool useTransferBuffer;
		VkBuffer buffer;
		VkDeviceMemory memory;
		VkDescriptorBufferInfo bufferInfo;
};
