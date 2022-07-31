#pragma once

#include <vulkan/vulkan.h>

#include "buffer.h"


class Device;

class Descriptor {
	public:
		Descriptor(const Device* device);
		~Descriptor();

		void init();
		void bind(const VkCommandBuffer* commandBuffer);

		Buffer& getBuffer();

		VkDescriptorPool descriptorPool;
		VkDeviceSize bufferSize;
		uint32_t bindingSetIndex;
		uint32_t dstBinding;
		VkDescriptorType descriptorType;
		VkDescriptorSetLayout setLayout;

		Buffer* buffer;

	private:
		void createBuffer();
		void createDescriptorSet();

		const Device* device;

		VkDescriptorSet descriptorSet;
};
