#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <vector>
#include <fstream>

#include "device.h"


class ComputePipeline {
	public:
		ComputePipeline(Device* device);
		~ComputePipeline();

		void init();
		void cmdExecutePipeline(const VkCommandBuffer* commandBuffer);

		std::string shaderPath;
		VkPipelineLayout pipelineLayout;
		uint32_t x, y, z;
	
	private:
		void createPipeline();

		Device* device;

		VkPipeline pipeline;
};
