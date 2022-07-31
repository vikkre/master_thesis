#pragma once

#include <vulkan/vulkan.h>

#include "data_buffer.h"


class Device;

class Descriptor {
	public:
		Descriptor(const Device* device);
		~Descriptor();

		void init();
		void bind(const VkCommandBuffer* commandBuffer);

		DataBuffer& getBuffer();

		VkDescriptorPool descriptorPool;
		VkDeviceSize bufferSize;
		uint32_t bindingSetIndex;
		uint32_t dstBinding;
		VkDescriptorType descriptorType;
		VkDescriptorSetLayout setLayout;

		DataBuffer* buffer;

	private:
		void createBuffer();
		void createDescriptorSet();

		const Device* device;

		VkDescriptorSet descriptorSet;
};
