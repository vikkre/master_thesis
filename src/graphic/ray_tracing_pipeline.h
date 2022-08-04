#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <string>
#include <fstream>

#include "helper/data_buffer.h"

#include "../init_exception.h"


class Device;


class RayTracingPipeline {
	public:
		RayTracingPipeline(Device* device);
		~RayTracingPipeline();

		void init();
		void cmdExecutePipeline(const VkCommandBuffer* commandBuffer);

		static void cmdRayTracingBarrier(const VkCommandBuffer* commandBuffer);

		std::vector<std::string> raygenShaders, missShaders, hitShaders;

		uint32_t width, height, depth;
		VkPipelineLayout pipelineLayout;

	private:
		void getProperties();
		void createShaderBindingTable();
		void createRayTracingPipeline();

		VkRayTracingShaderGroupCreateInfoKHR createShaderGroup();

		uint32_t alignedSize(uint32_t value, uint32_t alignment);

		Device* device;

		VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties;

		DataBuffer raygenShaderBindingTable;
		DataBuffer missShaderBindingTable;
		DataBuffer hitShaderBindingTable;

		VkPipeline pipeline;
};
