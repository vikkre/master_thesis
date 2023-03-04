#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <unordered_map>

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
		void addBuffer(uint32_t bindingIndex, BufferDescriptor* bufferDescriptor);
	
	private:
		void createDescriptorSetLayout();
		void createDescriptorPool();
		void createDescriptorSets();

		Device* device;

		VkDescriptorSetLayout descriptorSetLayout;
		VkDescriptorPool descriptorPool;
		std::vector<VkDescriptorSet> descriptorSets;

		std::unordered_map<uint32_t, BufferDescriptor*> bufferDescriptorsNew;
};
