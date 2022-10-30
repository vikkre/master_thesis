#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "../camera.h"


class ImageBuffer;

struct RenderInfo {
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	std::vector<VkSurfaceFormatKHR> surfaceFormats;
	std::vector<VkPresentModeKHR> presentModes;

	uint32_t swapchainImageCount;
	VkRenderPass swapchainRenderPass;
	VkExtent2D swapchainExtend;
	VkFormat swapchainImageFormat;
	VkFormat swapchainDepthFormat;
	std::vector<ImageBuffer*> swapchainImages;

	Camera camera;
};
