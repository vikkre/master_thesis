#pragma once

#include <vulkan/vulkan.h>

#include "../../init_exception.h"


class Device;

class Buffer {
	public:
		Buffer(const Device* device);
		~Buffer();

		void init();
		void passData(void* data);
		void passData(void* data, size_t start, size_t length);
		void getData(void* data);
		void getData(void* data, size_t start, size_t length);

		const VkBuffer& getBuffer() const;
		VkDeviceAddress getAddress() const;

		VkDeviceSize bufferSize;
		VkBufferUsageFlags usage;
		VkMemoryPropertyFlags properties;

	private:
		void createBuffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& dstBuffer, VkDeviceMemory& dstMemory);

		const Device* device;

		bool useTransferBuffer;
		VkBuffer buffer;
		VkDeviceMemory memory;
};
