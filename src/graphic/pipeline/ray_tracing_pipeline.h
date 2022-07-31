#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "pipeline.h"
#include "../helper/data_buffer.h"
#include "../helper/image_buffer.h"
#include "../helper/top_acceleration_structure_buffer.h"
#include "../helper/buffer_descriptor.h"
#include "../helper/single_buffer_descriptor.h"
#include "../helper/multi_buffer_descriptor.h"

#include "../../math/vector.h"
#include "../../math/matrix.h"
#include "../../init_exception.h"


class Device;
class GraphicsObject;


class RayTracingPipeline: public Pipeline {
	public:
		struct GlobalData {
			Matrix4f viewInverse;
			Matrix4f projInverse;
			Matrix4f view;
			Matrix4f proj;
			Vector3f backgroundColor;
			Vector3f lightPosition;
		};

		RayTracingPipeline(Device* device);
		~RayTracingPipeline() override;

		virtual void init() override;
		void initTlas();
		virtual void recordPreRenderCommandBuffer(size_t index, const VkCommandBuffer* commandBuffer) override;
		virtual void updateUniforms(size_t index) override;

		virtual const VkPipelineLayout& getPipelineLayout() const override;
		virtual const VkPipeline& getGraphicsPipeline() const override;

		static uint32_t getBindingSet();

		std::vector<GraphicsObject*> objects;
		GlobalData globalData;

	private:
		void getProperties();
		void createDescriptorSetLayout();
		void createStorageImages();
		void createShaderBindingTable();
		void createDescriptorPool();
		void createBuffers();
		void createDescriptorSets();
		void createRayTracingPipeline();

		VkRayTracingShaderGroupCreateInfoKHR createShaderGroup();

		uint32_t alignedSize(uint32_t value, uint32_t alignment);

		VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties;

		TopAccelerationStructureBuffer tlas;
		DataBuffer raygenShaderBindingTable;
		DataBuffer missShaderBindingTable;
		DataBuffer hitShaderBindingTable;
		std::vector<ImageBuffer> storageImages;

		VkPipeline pipeline;
		VkPipelineLayout pipelineLayout;
		VkDescriptorPool descriptorPool;
		std::vector<DataBuffer> globalDataBuffers;
		std::vector<DataBuffer> rtDataBuffers;
		std::vector<void*> rtDataPtrs;
		std::vector<VkDescriptorSet> descriptorSets;
		std::vector<BufferDescriptor*> bufferDescriptors; 

		typedef std::vector<char> ShaderCode;
		std::vector<ShaderCode> raygenShaders, missShaders, hitShaders;
};
