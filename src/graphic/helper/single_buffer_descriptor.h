#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "buffer.h"
#include "buffer_descriptor.h"


template<typename BufferType>
class SingleBufferDescriptor: public BufferDescriptor {
	public:
		SingleBufferDescriptor(Device* device)
		:bufferProperties(), buffer(device) {}

		~SingleBufferDescriptor() {}

		void init() {
			buffer.properties = bufferProperties;
			buffer.init();
		}

		virtual void getWriteDescriptorSets(std::vector<VkWriteDescriptorSet>& writeDescriptorSets, const std::vector<VkDescriptorSet>& descriptorSets, uint32_t binding) const override {
			for (VkDescriptorSet descriptorSet: descriptorSets) {
				writeDescriptorSets.push_back(buffer.getWriteDescriptorSet(descriptorSet, binding));
			}
		}

		virtual VkDescriptorSetLayoutBinding getLayoutBinding(uint32_t binding) const override {
			VkDescriptorSetLayoutBinding layoutBinding{};

			layoutBinding.binding = binding;
			layoutBinding.descriptorType = buffer.getDescriptorType();
			layoutBinding.descriptorCount = 1;
			layoutBinding.stageFlags = VK_SHADER_STAGE_ALL;

			return layoutBinding;
		}

		virtual const Buffer* getBuffer() const override {
			return &buffer;
		}

		typename BufferType::Properties bufferProperties;
	
	private:
		BufferType buffer;
};
