#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "pipeline.h"
#include "../helper/buffer.h"
#include "../helper/image.h"
#include "../helper/top_acceleration_structure.h"

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

		static std::vector<VkDescriptorSetLayoutBinding> getUniformBindings();
		static uint32_t getBindingSet();

		std::vector<GraphicsObject*> objects;
		GlobalData globalData;

	private:
		void getProperties();
		void createDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& uniformBindings, VkDescriptorSetLayout* pSetLayout);
		void createStorageImages();
		void createShaderBindingTable();
		void createDescriptorPool();
		void createBuffers();
		void createDescriptorSets();
		void createRayTracingPipeline();

		uint32_t alignedSize(uint32_t value, uint32_t alignment);

		VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties;

		TopAccelerationStructure tlas;
		Buffer raygenShaderBindingTable;
		Buffer missShaderBindingTable;
		Buffer hitShaderBindingTable;
		std::vector<Image> storageImages;
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups;

		VkPipeline pipeline;
		VkPipelineLayout pipelineLayout;
		VkDescriptorPool descriptorPool;
		std::vector<Buffer> globalDataBuffers;
		std::vector<Buffer> rtDataBuffers;
		std::vector<void*> rtDataPtrs;
		std::vector<VkDescriptorSet> descriptorSets;

		std::vector<char> rgenCode, rmissCode, rshadowCode, rchitCode;
};
