#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "../camera.h"
#include "../pipeline/pipeline.h"


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

	Pipeline* renderPipeline;
	VkDescriptorSetLayout globalDescriptorSetLayout;
	VkDescriptorSetLayout objectDescriptorSetLayout;

	Camera camera;
	Vector3f backgroundColor;
	Vector3f lightPosition;
};
