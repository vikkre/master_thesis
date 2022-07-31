#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "buffer.h"
#include "buffer_descriptor.h"

#include "../../init_exception.h"


class MultiBufferDescriptor: public BufferDescriptor {
	public:
		MultiBufferDescriptor(std::vector<Buffer*> buffers, uint32_t binding, VkShaderStageFlags shaderStageFlags);
		~MultiBufferDescriptor();

		virtual void getWriteDescriptorSets(std::vector<VkWriteDescriptorSet>& writeDescriptorSets, const std::vector<VkDescriptorSet>& descriptorSets) const override;
		virtual VkDescriptorSetLayoutBinding getLayoutBinding() const override;
		virtual const Buffer* getBuffer() const override;

		template <typename T>
		static std::vector<Buffer*> vectorToBufferPointer(std::vector<T>& vector) {
			std::vector<Buffer*> ptrVector(vector.size());

			for (unsigned int i = 0; i < vector.size(); ++i) {
				ptrVector[i] = &vector[i];
			}

			return ptrVector;
		}
	
	private:
		std::vector<Buffer*> buffers;
		uint32_t binding;
		VkShaderStageFlags shaderStageFlags;
};
