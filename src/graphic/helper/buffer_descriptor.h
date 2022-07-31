#pragma once

#include <vulkan/vulkan.h>


class BufferDescriptor {
	public:
		BufferDescriptor() {}
		virtual ~BufferDescriptor() {}

		virtual void getWriteDescriptorSets(std::vector<VkWriteDescriptorSet>& writeDescriptorSets, const std::vector<VkDescriptorSet>& descriptorSets) const=0;
		virtual VkDescriptorSetLayoutBinding getLayoutBinding() const=0;
		virtual const Buffer* getBuffer() const=0;
};
