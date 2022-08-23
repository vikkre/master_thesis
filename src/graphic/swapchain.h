#pragma once

#include <vulkan/vulkan.h>

#include <algorithm>
#include <functional>

#include "frame_buffer.h"
#include "helper/image_buffer.h"
#include "helper/multi_buffer_descriptor.h"

#include "../init_exception.h"


class Device;

class Swapchain {
	public:
		Swapchain(Device* device);
		~Swapchain();

		void init();
		void recordCommandBuffers(std::function<void(size_t, VkCommandBuffer)> recordCommandBuffer);
		void render(std::function<void(size_t)> updateUniform);
		MultiBufferDescriptor<ImageBuffer>* getInputImageBuffer();

		const FrameBuffer& getFrame(size_t index) const;

		size_t getCurrentFrame() const;
		
		VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		VkFormat findDepthFormat();

	private:
		void createSwapChain();
		void createImageViews();
		void createRenderPass();
		void createSyncObjects();
		void createInputImages();

		Device* device;

		VkExtent2D swapchainExtend;
		VkFormat swapchainImageFormat;
		VkSwapchainKHR swapchain;

		MultiBufferDescriptor<ImageBuffer> inputImages;
		std::vector<FrameBuffer> frames;

		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;
		std::vector<VkFence> imagesInFlight;
		size_t currentFrame;
};
