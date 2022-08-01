#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <fstream>

#include "../../math/vector.h"
#include "../../init_exception.h"


class Device;

class Pipeline {
	public:
		Pipeline(Device* device): device(device) {}
		virtual ~Pipeline() {}

		virtual void init()=0;
		// virtual void updateUniforms(size_t /* index */) {};
		
		// virtual void recordPreRenderCommandBuffer (size_t /* index */, const VkCommandBuffer* /* commandBuffer */) {}
		// virtual void recordRenderCommandBuffer    (size_t /* index */, const VkCommandBuffer* /* commandBuffer */) {}
		// virtual void recordPostRenderCommandBuffer(size_t /* index */, const VkCommandBuffer* /* commandBuffer */) {}

		virtual const VkPipelineLayout& getPipelineLayout() const=0;
		virtual const VkPipeline& getGraphicsPipeline() const=0;

	protected:
		Device* device;
};


std::vector<char> readFile(const std::string& filename);
VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& code);
