#pragma once

#include <vulkan/vulkan.h>

#include "../../input_parser.h"


class GraphicsObject;

class Renderer {
	public:
		struct GlobalData {
			Matrix4f viewInverse;
			Matrix4f projInverse;
			Matrix4f view;
			Matrix4f proj;
		} globalData;

		Renderer(): globalData(), objects(), outputImages(nullptr) {}
		virtual ~Renderer() {}

		virtual void init()=0;
		virtual void cmdRender(size_t index, VkCommandBuffer commandBuffer)=0;
		virtual void updateUniforms(size_t index)=0;
		virtual void parseInput(const InputEntry& inputEntry)=0;

		void passObjects(const std::vector<GraphicsObject*>& objects) { this->objects = objects; }
		void setOutputImageBuffer(MultiBufferDescriptor<ImageBuffer>* outputImageBuffer) { outputImages = outputImageBuffer; }

		static void cmdPipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkCommandBuffer commandBuffer) {
			vkCmdPipelineBarrier(
				commandBuffer,
				srcStageMask, dstStageMask,
				0,
				0, nullptr,
				0, nullptr,
				0, nullptr
			);
		}

	protected:
		std::vector<GraphicsObject*> objects;
		MultiBufferDescriptor<ImageBuffer>* outputImages;
};
