#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "buffer.h"
#include "buffer_descriptor.h"


class SingleBufferDescriptor: public BufferDescriptor {
	public:
		SingleBufferDescriptor(Buffer* buffer, uint32_t binding, VkShaderStageFlags shaderStageFlags);
		~SingleBufferDescriptor();

		virtual void getWriteDescriptorSets(std::vector<VkWriteDescriptorSet>& writeDescriptorSets, const std::vector<VkDescriptorSet>& descriptorSets) const override;
		virtual VkDescriptorSetLayoutBinding getLayoutBinding() const override;
		virtual const Buffer* getBuffer() const override;
	
	private:
		Buffer* buffer;
		uint32_t binding;
		VkShaderStageFlags shaderStageFlags;
};
