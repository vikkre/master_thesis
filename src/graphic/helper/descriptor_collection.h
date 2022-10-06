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
		void cmdBind(size_t index, VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const;
		VkDescriptorSetLayout getLayout() const;

		unsigned int bindingSetIndex;
		std::vector<BufferDescriptor*> bufferDescriptors;
	
	private:
		void createDescriptorSetLayout();
		void createDescriptorPool();
		void createDescriptorSets();

		Device* device;

		VkDescriptorSetLayout descriptorSetLayout;
		VkDescriptorPool descriptorPool;
		std::vector<VkDescriptorSet> descriptorSets;
};
