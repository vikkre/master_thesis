#include "image.h"

#include "../device.h"
#include "command_buffer.h"


Image::Image(Device* device)
:width(), height(), format(), tiling(), usage(), properties(), aspectFlags(),
layout(VK_IMAGE_LAYOUT_UNDEFINED),
device(device), image(VK_NULL_HANDLE), imageMemory(VK_NULL_HANDLE), imageView(VK_NULL_HANDLE),
deleteImage(true) {}

Image::~Image() {
	if (imageView != VK_NULL_HANDLE) vkDestroyImageView(device->getDevice(), imageView, nullptr);
	if (imageMemory != VK_NULL_HANDLE) vkFreeMemory(device->getDevice(), imageMemory, nullptr);
	if (deleteImage) vkDestroyImage(device->getDevice(), image, nullptr);
}

void Image::init() {
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateImage(device->getDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS) {
		throw InitException("vkCreateImage", "failed to create image!");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device->getDevice(), image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = device->findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(device->getDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
		throw InitException("vkAllocateMemory", "failed to allocate image memory!");
	}

	vkBindImageMemory(device->getDevice(), image, imageMemory, 0);

	if (layout != VK_IMAGE_LAYOUT_UNDEFINED) {
		VkImageLayout wantedLayout = layout;
		layout = VK_IMAGE_LAYOUT_UNDEFINED;

		SingleUseCommandBuffer commandBuffer(device, &device->getQueues().getGraphicsQueue());
		commandBuffer.start();
		Image::transitionImageLayout(&commandBuffer.getCommandBuffer(), wantedLayout);
		commandBuffer.end();
	}
}

void Image::init(VkImage image) {
	this->image = image;
	deleteImage = false;
	layout = VK_IMAGE_LAYOUT_UNDEFINED;

	SingleUseCommandBuffer commandBuffer(device, &device->getQueues().getGraphicsQueue());
	commandBuffer.start();
	Image::transitionImageLayout(&commandBuffer.getCommandBuffer(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	commandBuffer.end();
}

void Image::createImageView() {
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	if (vkCreateImageView(device->getDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
		throw InitException("vkCreateImageView", "failed to create image view!");
	}
}

void Image::copyImage(const VkCommandBuffer* commandBuffer, Image* destination) {
	VkImageLayout oldSrcLayout = (this->layout == VK_IMAGE_LAYOUT_UNDEFINED ? VK_IMAGE_LAYOUT_GENERAL : this->layout);
	VkImageLayout oldDstLayout = (destination->layout == VK_IMAGE_LAYOUT_UNDEFINED ? VK_IMAGE_LAYOUT_GENERAL : destination->layout);

	this->transitionImageLayout(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	destination->transitionImageLayout(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	VkImageCopy copyRegion{};
	copyRegion.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	copyRegion.srcOffset = { 0, 0, 0 };
	copyRegion.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	copyRegion.dstOffset = { 0, 0, 0 };
	copyRegion.extent = { width, height, 1 };
	vkCmdCopyImage(
		*commandBuffer,
		this->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		destination->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1, &copyRegion
	);

	this->transitionImageLayout(commandBuffer, oldSrcLayout);
	destination->transitionImageLayout(commandBuffer, oldDstLayout);
}

void Image::transitionImageLayout(const VkCommandBuffer* commandBuffer, VkImageLayout newLayout) {
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = layout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;

	vkCmdPipelineBarrier(
		*commandBuffer,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	layout = newLayout;
}


const VkImage& Image::getImage() const {
	return image;
}

const VkDeviceMemory& Image::getMemory() const {
	return imageMemory;
}

const VkImageView& Image::getImageView() const {
	return imageView;
}
