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
		void cmdExecutePipeline(VkCommandBuffer commandBuffer);

		static VkPipelineStageFlags getStageMask();

		std::vector<std::string> raygenShaders, missShaders, hitShaders;

		uint32_t width, height, depth;
		VkPipelineLayout pipelineLayout;

	private:
		void getProperties();
		void createShaderBindingTable();
		void createRayTracingPipeline();

		VkRayTracingShaderGroupCreateInfoKHR createShaderGroup();
		std::vector<char> readFile(const std::string& filename);
		VkShaderModule createShaderModule(const std::vector<char>& code);

		uint32_t alignedSize(uint32_t value, uint32_t alignment);

		Device* device;

		VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties;

		DataBuffer raygenShaderBindingTable;
		DataBuffer missShaderBindingTable;
		DataBuffer hitShaderBindingTable;

		VkPipeline pipeline;
};
