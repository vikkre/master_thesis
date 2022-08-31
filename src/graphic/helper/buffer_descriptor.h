#pragma once

#include <vulkan/vulkan.h>

#include "buffer.h"


class BufferDescriptor {
	public:
		BufferDescriptor() {}
		virtual ~BufferDescriptor() {}

		virtual void getWriteDescriptorSets(std::vector<VkWriteDescriptorSet>& writeDescriptorSets, const std::vector<VkDescriptorSet>& descriptorSets, uint32_t binding) const=0;
		virtual VkDescriptorSetLayoutBinding getLayoutBinding(uint32_t binding) const=0;
		virtual const Buffer* getBuffer() const=0;
};
