#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "../device.h"
#include "buffer.h"
#include "buffer_descriptor.h"

#include "../../init_exception.h"


template<typename BufferType>
class MultiBufferDescriptor: public BufferDescriptor {
	public:
		MultiBufferDescriptor(Device* device)
		:bufferProperties(), device(device), buffers() {}

		~MultiBufferDescriptor() {}

		void init() {
			buffers.resize(device->renderInfo.swapchainImageCount, device);
			for (size_t i = 0; i < device->renderInfo.swapchainImageCount; ++i) {
				buffers[i].properties = bufferProperties;
				buffers[i].init();
			}
		}

		virtual void getWriteDescriptorSets(std::vector<VkWriteDescriptorSet>& writeDescriptorSets, const std::vector<VkDescriptorSet>& descriptorSets, uint32_t binding) const override {
			if (descriptorSets.size() != buffers.size()) {
				throw InitException("MultiBufferDescriptor::getWriteDescriptorSets", "descriptorSets.size() and buffers.size() are unequal!");
			}
			
			for (unsigned int i = 0; i < buffers.size(); ++i) {
				writeDescriptorSets.push_back(buffers[i].getWriteDescriptorSet(descriptorSets[i], binding));
			}
		}

		virtual VkDescriptorSetLayoutBinding getLayoutBinding(uint32_t binding) const override {
			VkDescriptorSetLayoutBinding layoutBinding{};

			layoutBinding.binding = binding;
			layoutBinding.descriptorType = buffers[0].getDescriptorType();
			layoutBinding.descriptorCount = 1;
			layoutBinding.stageFlags = VK_SHADER_STAGE_ALL;

			return layoutBinding;
		}

		virtual const Buffer* getBuffer() const override {
			return &buffers[0];
		}

		BufferType& at(unsigned int index) {
			return buffers.at(index);
		}

		typename BufferType::Properties bufferProperties;
	
	private:
		Device* device;
		std::vector<BufferType> buffers;
};
