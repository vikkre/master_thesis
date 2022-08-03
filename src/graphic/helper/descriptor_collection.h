#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "../device.h"
#include "buffer_descriptor.h"


class DescriptorCollection {
	public:
		DescriptorCollection(Device* device);
		~DescriptorCollection();

		void init();
		void cmdBind(size_t index, const VkCommandBuffer* commandBuffer) const;
		VkPipelineLayout getPipelineLayout() const;

		std::vector<BufferDescriptor*> bufferDescriptors;
	
	private:
		void createDescriptorSetLayout();
		void createDescriptorPool();
		void createDescriptorSets();
		void createPipelineLayout();

		Device* device;

		VkDescriptorSetLayout descriptorSetLayout;
		VkDescriptorPool descriptorPool;
		std::vector<VkDescriptorSet> descriptorSets;
		VkPipelineLayout pipelineLayout;
};
