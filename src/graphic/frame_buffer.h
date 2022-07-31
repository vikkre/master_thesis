#pragma once

#include <vulkan/vulkan.h>

#include "../init_exception.h"

#include "helper/image_buffer.h"


class Device;

class FrameBuffer {
	public:
		FrameBuffer(Device* device);
		~FrameBuffer();

		void init(const VkImage& image);
		void recordCommandBuffer(size_t index);

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
