#pragma once

#include <vulkan/vulkan.h>


class Buffer {
	public:
		Buffer() {}
		virtual ~Buffer() {}

		virtual VkWriteDescriptorSet getWriteDescriptorSet(VkDescriptorSet descriptorSet, uint32_t binding) const=0;
		virtual VkDescriptorType getDescriptorType() const=0;
};
