#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <string>
#include <fstream>

#include "helper/data_buffer.h"
#include "helper/buffer_descriptor.h"

#include "../init_exception.h"


class Device;
class GraphicsObject;


class RayTracingPipeline {
	public:
		RayTracingPipeline(Device* device);
		~RayTracingPipeline();

		void init();
		void cmdExecutePipeline(size_t index, const VkCommandBuffer* commandBuffer);

		std::vector<std::string> raygenShaders, missShaders, hitShaders;
		std::vector<BufferDescriptor*> bufferDescriptors;

		uint32_t width, height;

	private:
		void getProperties();
		void createDescriptorSetLayout();
		void createShaderBindingTable();
		void createDescriptorPool();
		void createDescriptorSets();
		void createRayTracingPipeline();

		VkRayTracingShaderGroupCreateInfoKHR createShaderGroup();

		uint32_t alignedSize(uint32_t value, uint32_t alignment);

		Device* device;

		VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties;

		DataBuffer raygenShaderBindingTable;
		DataBuffer missShaderBindingTable;
		DataBuffer hitShaderBindingTable;

		VkPipeline pipeline;
		VkPipelineLayout pipelineLayout;
		VkDescriptorPool descriptorPool;
		std::vector<VkDescriptorSet> descriptorSets;
		VkDescriptorSetLayout descriptorSetLayout;
};
