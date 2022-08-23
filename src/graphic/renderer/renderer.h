#pragma once

#include <vulkan/vulkan.h>


class Renderer {
	public:
		struct GlobalData {
			Matrix4f viewInverse;
			Matrix4f projInverse;
			Matrix4f view;
			Matrix4f proj;
		} globalData;

		Renderer(): globalData(), outputImages(nullptr) {}
		virtual ~Renderer() {}

		virtual void init()=0;
		virtual void cmdRender(size_t index, VkCommandBuffer commandBuffer)=0;
		virtual void updateUniforms(size_t index)=0;

		void setOutputImageBuffer(MultiBufferDescriptor<ImageBuffer>* outputImageBuffer) { outputImages = outputImageBuffer; }

	protected:
		MultiBufferDescriptor<ImageBuffer>* outputImages;
};
