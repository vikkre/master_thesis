#include "swapchain.h"

#include "device.h"


#define MAX_FRAMES_IN_FLIGHT 2


// Adds
uint32_t special_clamp(uint32_t value, uint32_t min, uint32_t max) {
	if (value < min) return min;
	if (value > max) return max;
	return value;
}

VkSurfaceFormatKHR special_chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	for (const VkSurfaceFormatKHR& availableFormat: availableFormats) {
		// if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
		// 	return availableFormat;
		// }
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	return availableFormats.at(0);
}

VkPresentModeKHR special_chooseSwapPresentMode(const std::vector<VkPresentModeKHR>&) {
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D special_chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D windowExtend) {
	if (capabilities.currentExtent.width != UINT32_MAX) {
		return capabilities.currentExtent;
	} else {
		VkExtent2D actualExtend = windowExtend;

		actualExtend.width = special_clamp(actualExtend.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtend.height = special_clamp(actualExtend.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtend;
	}
}


// Swapchain
Swapchain::Swapchain(Device* device)
:device(device),
swapchainExtend(), swapchainImageFormat(), swapchain(VK_NULL_HANDLE), inputImages(device),
frames(), currentFrame(0) {}

Swapchain::~Swapchain() {
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(device->getDevice(), renderFinishedSemaphores.at(i), nullptr);
		vkDestroySemaphore(device->getDevice(), imageAvailableSemaphores.at(i), nullptr);
		vkDestroyFence(device->getDevice(), inFlightFences.at(i), nullptr);
	}

	frames.clear();
	vkDestroyRenderPass(device->getDevice(), device->renderInfo.swapchainRenderPass, nullptr);
	vkDestroySwapchainKHR(device->getDevice(), swapchain, nullptr);
}

void Swapchain::init() {
	createSwapChain();
	createRenderPass();
	createImageViews();
	createSyncObjects();
	createInputImages();
}

void Swapchain::recordCommandBuffers(std::function<void(size_t, VkCommandBuffer)> recordCommandBuffer) {
	for (size_t i = 0; i < device->renderInfo.swapchainImageCount; ++i) {
		frames.at(i).recordCommandBuffer(recordCommandBuffer, i, inputImages.at(i));
	}
}

void Swapchain::render(std::function<void(size_t)> updateUniform) {
	vkWaitForFences(device->getDevice(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	vkAcquireNextImageKHR(device->getDevice(), swapchain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

	if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
		vkWaitForFences(device->getDevice(), 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
	}
	imagesInFlight[imageIndex] = inFlightFences[currentFrame];

	updateUniform(imageIndex);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &frames.at(imageIndex).getCommandBuffer();

	VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	vkResetFences(device->getDevice(), 1, &inFlightFences[currentFrame]);

	if (vkQueueSubmit(device->getQueues().getGraphicsQueue().queue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = {swapchain};
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;

	VkResult queuePresentResult = vkQueuePresentKHR(device->getQueues().getPresentQueue().queue, &presentInfo);

	if (queuePresentResult != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}

	vkWaitForFences(device->getDevice(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
		vkWaitForFences(device->getDevice(), 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
	}

	inputImages.at(currentFrame).saveImageAsNetpbm("example.ppm");

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

MultiBufferDescriptor<ImageBuffer>* Swapchain::getInputImageBuffer() {
	return &inputImages;
}

void Swapchain::createSwapChain() {
	VkSurfaceFormatKHR surfaceFormat = special_chooseSwapSurfaceFormat(device->renderInfo.surfaceFormats);
	VkPresentModeKHR presentMode = special_chooseSwapPresentMode(device->renderInfo.presentModes);
	swapchainExtend = special_chooseSwapExtent(device->renderInfo.surfaceCapabilities, device->window->getWindowExtend());
	device->renderInfo.swapchainExtend = swapchainExtend;

	uint32_t imageCount = device->renderInfo.surfaceCapabilities.minImageCount + 1;

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = device->window->getSurface();

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = swapchainExtend;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	uint32_t queueFamilyIndices[] = {
		device->getQueues().getGraphicsQueue().queueIndex,
		device->getQueues().getPresentQueue().queueIndex,
	};

	if (queueFamilyIndices[0] != queueFamilyIndices[1]) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	} else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	createInfo.preTransform = device->renderInfo.surfaceCapabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(device->getDevice(), &createInfo, nullptr, &swapchain) != VK_SUCCESS) {
		throw InitException("vkCreateSwapchainKHR", "failed to create swap chain!");
	}

	swapchainImageFormat = surfaceFormat.format;
	device->renderInfo.swapchainImageFormat = swapchainImageFormat;
	device->renderInfo.swapchainDepthFormat = findDepthFormat();
}

void Swapchain::createImageViews() {
	device->renderInfo.swapchainImageCount = 0;
	vkGetSwapchainImagesKHR(device->getDevice(), swapchain, &device->renderInfo.swapchainImageCount, nullptr);
	std::vector<VkImage> swapChainImages(device->renderInfo.swapchainImageCount);
	vkGetSwapchainImagesKHR(device->getDevice(), swapchain, &device->renderInfo.swapchainImageCount, swapChainImages.data());

	frames.clear();
	frames.reserve(device->renderInfo.swapchainImageCount);

	for (size_t i = 0; i < device->renderInfo.swapchainImageCount; ++i) {
		frames.emplace_back(device);
		frames.at(i).init(swapChainImages.at(i));
	}
}

void Swapchain::createRenderPass() {
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = swapchainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = device->renderInfo.swapchainDepthFormat;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	std::vector<VkAttachmentDescription> attachments = {colorAttachment, depthAttachment};

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = (uint32_t) attachments.size();
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(device->getDevice(), &renderPassInfo, nullptr, &device->renderInfo.swapchainRenderPass) != VK_SUCCESS) {
		throw InitException("vkCreateRenderPass", "failed to create render pass!");
	}
}

void Swapchain::createSyncObjects() {
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	imagesInFlight.resize(device->renderInfo.swapchainImageCount, VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(device->getDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(device->getDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(device->getDevice(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
			throw InitException("createSyncObjects", "failed to create synchronization objects for a frame!");
		}
	}
}

void Swapchain::createInputImages() {
	inputImages.bufferProperties.width = swapchainExtend.width;
	inputImages.bufferProperties.height = swapchainExtend.height;
	inputImages.bufferProperties.format = swapchainImageFormat;
	inputImages.bufferProperties.tiling = VK_IMAGE_TILING_OPTIMAL;
	inputImages.bufferProperties.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
	inputImages.bufferProperties.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	inputImages.bufferProperties.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
	inputImages.bufferProperties.layout = VK_IMAGE_LAYOUT_GENERAL;
	inputImages.bufferProperties.createImageView = true;
	inputImages.init();
}


VkFormat Swapchain::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
	for (VkFormat format: candidates) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(device->getPhysicalDevice(), format, &props);

		if        (tiling == VK_IMAGE_TILING_LINEAR  && (props.linearTilingFeatures  & features) == features) {
			return format;
		} else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}

	throw InitException("Frame::findSupportedFormat", "failed to find supported format!");
}

VkFormat Swapchain::findDepthFormat() {
	return findSupportedFormat(
		{VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}


const FrameBuffer& Swapchain::getFrame(size_t index) const {
	return frames.at(index);
}

size_t Swapchain::getCurrentFrame() const {
	return currentFrame;
}
