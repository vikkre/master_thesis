#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <string>

#include "pipeline.h"
#include "../helper/data_buffer.h"
#include "../helper/buffer_descriptor.h"

#include "../../init_exception.h"


class Device;
class GraphicsObject;


class RayTracingPipeline: public Pipeline {
	public:
		RayTracingPipeline(Device* device);
		~RayTracingPipeline() override;

		virtual void init() override;
		void cmdExecutePipeline(size_t index, const VkCommandBuffer* commandBuffer);

		virtual const VkPipelineLayout& getPipelineLayout() const override;
		virtual const VkPipeline& getGraphicsPipeline() const override;

		static uint32_t getBindingSet();

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
