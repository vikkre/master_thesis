#include "frame_buffer.h"

#include "device.h"


FrameBuffer::FrameBuffer(Device* device)
:device(device),
image(device), depthImage(device),
frameBuffer(VK_NULL_HANDLE), renderCommandBuffer(VK_NULL_HANDLE) {}

FrameBuffer::~FrameBuffer() {
	vkFreeCommandBuffers(device->getDevice(), device->getQueues().getGraphicsQueue().commandPool, 1, &renderCommandBuffer);
	vkDestroyFramebuffer(device->getDevice(), frameBuffer, nullptr);
}

void FrameBuffer::init(const VkImage& image) {
	this->image.format = device->renderInfo.swapchainImageFormat;
	this->image.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
	this->image.init(image, true);

	createDepthResources();
	createFramebuffer();
	createCommandBuffer();

	device->renderInfo.swapchainImages.push_back(&this->image);
}

void FrameBuffer::recordCommandBuffer(std::function<void(size_t, VkCommandBuffer*)> recordCommandBuffer, size_t index) {
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	Vector3f& clearColor = device->renderInfo.backgroundColor;
	std::vector<VkClearValue> clearValues = {
		{clearColor[0], clearColor[1], clearColor[2], 1.0f},
		{1.0f, 0},
	};

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = device->renderInfo.swapchainRenderPass;
	renderPassInfo.framebuffer = frameBuffer;
	renderPassInfo.renderArea.offset = {0, 0};
	renderPassInfo.renderArea.extent = device->renderInfo.swapchainExtend;
	renderPassInfo.clearValueCount = (uint32_t) clearValues.size();
	renderPassInfo.pClearValues = clearValues.data();

	if (vkBeginCommandBuffer(renderCommandBuffer, &beginInfo) != VK_SUCCESS) {
		throw InitException("vkBeginCommandBuffer", "failed to begin recording render command buffer!");
	}

	recordCommandBuffer(index, &renderCommandBuffer);

	if (vkEndCommandBuffer(renderCommandBuffer) != VK_SUCCESS) {
		throw InitException("vkEndCommandBuffer", "failed to record render command buffer!");
	}
}

void FrameBuffer::createDepthResources() {
	depthImage.width = device->renderInfo.swapchainExtend.width;
	depthImage.height = device->renderInfo.swapchainExtend.height;
	depthImage.format = device->renderInfo.swapchainDepthFormat;
	depthImage.tiling = VK_IMAGE_TILING_OPTIMAL;
	depthImage.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	depthImage.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	depthImage.aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
	depthImage.createImageView = true;

	depthImage.init();
}

void FrameBuffer::createFramebuffer() {
	std::vector<VkImageView> attachments = {
		image.getImageView(),
		depthImage.getImageView()
	};

	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = device->renderInfo.swapchainRenderPass;
	framebufferInfo.attachmentCount = (uint32_t) attachments.size();
	framebufferInfo.pAttachments = attachments.data();
	framebufferInfo.width = device->renderInfo.swapchainExtend.width;
	framebufferInfo.height = device->renderInfo.swapchainExtend.height;
	framebufferInfo.layers = 1;

	if (vkCreateFramebuffer(device->getDevice(), &framebufferInfo, nullptr, &frameBuffer) != VK_SUCCESS) {
		throw InitException("vkCreateFramebuffer", "failed to create framebuffer!");
	}
}

void FrameBuffer::createCommandBuffer() {
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = device->getQueues().getGraphicsQueue().commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(device->getDevice(), &allocInfo, &renderCommandBuffer) != VK_SUCCESS) {
		throw InitException("vkAllocateCommandBuffers", "failed to allocate render command buffer!");
	}
}

const ImageBuffer& FrameBuffer::getImage() const {
	return image;
}

const VkFramebuffer& FrameBuffer::getFrameBuffer() const {
	return frameBuffer;
}

const VkCommandBuffer& FrameBuffer::getCommandBuffer() const {
	return renderCommandBuffer;
}
