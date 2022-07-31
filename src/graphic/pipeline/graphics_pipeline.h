#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <fstream>

#include "pipeline.h"

#include "../../math/vector.h"
#include "../../math/matrix.h"
#include "../../init_exception.h"


class GraphicsObject;
class Descriptor;

class GraphicsPipeline: public Pipeline {
	public:
		struct GlobalData {
			Matrix4f viewMatrix;
			Matrix4f projectionMatrix;
			Vector3f lightPosition;
		};

		GraphicsPipeline(Device* device);
		~GraphicsPipeline() override;

		virtual void init() override;
		virtual void updateUniforms(size_t index) override;
		virtual void recordRenderCommandBuffer(size_t index, const VkCommandBuffer* commandBuffer) override;

		virtual const VkPipelineLayout& getPipelineLayout() const override;
		virtual const VkPipeline& getGraphicsPipeline() const override;

		static std::vector<VkDescriptorSetLayoutBinding> getUniformBindings();
		static uint32_t getBindingSet();

		std::vector<GraphicsObject*> objects;
		GlobalData globalData;

	private:
		void createDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& uniformBindings, VkDescriptorSetLayout* pSetLayout);
		void createGraphicsPipeline();
		void createDescriptorPool();
		void createDescriptors();

		VkPipelineLayout pipelineLayout;
		VkPipeline graphicsPipeline;
		VkDescriptorPool descriptorPool;
		std::vector<Descriptor> descriptors;

		std::vector<char> vertShaderCode, fragShaderCode;
};
