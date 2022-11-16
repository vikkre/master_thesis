#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <functional>

#include "../device.h"
#include "buffer.h"
#include "buffer_descriptor.h"
#include "image_buffer.h"

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
		
		size_t size() const {
			return buffers.size();
		}

		void forEach(std::function<void(BufferType&)> func) {
			for (size_t i = 0; i < device->renderInfo.swapchainImageCount; ++i) {
				func(buffers[i]);
			}
		}

		typename BufferType::Properties bufferProperties;
	
	private:
		Device* device;
		std::vector<BufferType> buffers;
		friend class MultiSamplerDescriptor;
};

class MultiSamplerDescriptor: public BufferDescriptor {
	public:
		MultiSamplerDescriptor(MultiBufferDescriptor<ImageBuffer>* imageBuffers)
		:BufferDescriptor(), imageBuffers(imageBuffers) {}

		~MultiSamplerDescriptor() {}

		virtual void getWriteDescriptorSets(std::vector<VkWriteDescriptorSet>& writeDescriptorSets, const std::vector<VkDescriptorSet>& descriptorSets, uint32_t binding) const override {
			if (descriptorSets.size() != imageBuffers->buffers.size()) {
				throw InitException("MultiSamplerDescriptor::getWriteDescriptorSets", "descriptorSets.size() and buffers.size() are unequal!");
			}
			
			for (unsigned int i = 0; i < imageBuffers->buffers.size(); ++i) {
				writeDescriptorSets.push_back(imageBuffers->buffers[i].getWriteDescriptorSetSampler(descriptorSets[i], binding));
			}
		}

		virtual VkDescriptorSetLayoutBinding getLayoutBinding(uint32_t binding) const override {
			VkDescriptorSetLayoutBinding layoutBinding{};

			layoutBinding.binding = binding;
			layoutBinding.descriptorType = imageBuffers->buffers[0].getDescriptorTypeSampler();
			layoutBinding.descriptorCount = 1;
			layoutBinding.stageFlags = VK_SHADER_STAGE_ALL;

			return layoutBinding;
		}

		virtual const Buffer* getBuffer() const override {
			return &imageBuffers->buffers[0];
		}
	
	private:
		MultiBufferDescriptor<ImageBuffer>* imageBuffers;
};

template<typename BufferType>
class MultiBufferDescriptorOffset: public BufferDescriptor {
	public:
		MultiBufferDescriptorOffset(Device* device, MultiBufferDescriptor<BufferType>* bufferDescriptor, unsigned int offset)
		:BufferDescriptor(), device(device), bufferDescriptor(bufferDescriptor), offset(offset) {}

		virtual void getWriteDescriptorSets(std::vector<VkWriteDescriptorSet>& writeDescriptorSets, const std::vector<VkDescriptorSet>& descriptorSets, uint32_t binding) const override {
			if (descriptorSets.size() != bufferDescriptor->size()) {
				throw InitException("MultiBufferDescriptor::getWriteDescriptorSets", "descriptorSets.size() and buffers.size() are unequal!");
			}
			
			for (unsigned int i = 0; i < bufferDescriptor->size(); ++i) {
				unsigned int ii = (i + offset) % device->renderInfo.swapchainImageCount;
				writeDescriptorSets.push_back(bufferDescriptor->at(ii).getWriteDescriptorSet(descriptorSets[i], binding));
			}
		}

		virtual VkDescriptorSetLayoutBinding getLayoutBinding(uint32_t binding) const override {
			return bufferDescriptor->getLayoutBinding(binding);
		}

		virtual const Buffer* getBuffer() const override {
			return bufferDescriptor->getBuffer();
		}
	private:
		Device* device;
		MultiBufferDescriptor<BufferType>* bufferDescriptor;
		unsigned int offset;
};
