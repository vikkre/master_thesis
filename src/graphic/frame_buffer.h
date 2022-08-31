#pragma once

#include <vulkan/vulkan.h>

#include <functional>

#include "../init_exception.h"

#include "helper/image_buffer.h"


class Device;

class FrameBuffer {
	public:
		FrameBuffer(Device* device);
		~FrameBuffer();

		void init(const VkImage& image);
		void recordCommandBuffer(std::function<void(size_t, VkCommandBuffer)> recordCommandBuffer, size_t index, ImageBuffer& inputImage);

		const ImageBuffer& getImage() const;
		const VkFramebuffer& getFrameBuffer() const;
		const VkCommandBuffer& getCommandBuffer() const;

	private:
		void createDepthResources();
		void createFramebuffer();
		void createCommandBuffer();

		Device* device;

		ImageBuffer image;
		ImageBuffer depthImage;

		VkFramebuffer frameBuffer;
		VkCommandBuffer renderCommandBuffer;
};
