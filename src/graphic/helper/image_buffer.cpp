#include "image_buffer.h"

#include "../device.h"
#include "command_buffer.h"


ImageBuffer::ImageBuffer(Device* device)
:Buffer(), properties(),
device(device), image(VK_NULL_HANDLE), imageMemory(VK_NULL_HANDLE),
imageView(VK_NULL_HANDLE), textureSampler(VK_NULL_HANDLE),
descriptorImageInfo(), deleteImage(true) {
	properties.layout = VK_IMAGE_LAYOUT_UNDEFINED;
	properties.createImageView = false;
	properties.createSampler = false;
}

ImageBuffer::~ImageBuffer() {
	if (textureSampler != VK_NULL_HANDLE) vkDestroySampler(device->getDevice(), textureSampler, nullptr);
	if (imageView != VK_NULL_HANDLE) vkDestroyImageView(device->getDevice(), imageView, nullptr);
	if (imageMemory != VK_NULL_HANDLE) vkFreeMemory(device->getDevice(), imageMemory, nullptr);
	if (deleteImage) vkDestroyImage(device->getDevice(), image, nullptr);
}

void ImageBuffer::init() {
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = properties.width;
	imageInfo.extent.height = properties.height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = properties.format;
	imageInfo.tiling = properties.tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = properties.usage;
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
	allocInfo.memoryTypeIndex = device->findMemoryType(memRequirements.memoryTypeBits, properties.properties);

	if (vkAllocateMemory(device->getDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
		throw InitException("vkAllocateMemory", "failed to allocate image memory!");
	}

	vkBindImageMemory(device->getDevice(), image, imageMemory, 0);

	if (properties.layout != VK_IMAGE_LAYOUT_UNDEFINED) {
		VkImageLayout wantedLayout = properties.layout;
		properties.layout = VK_IMAGE_LAYOUT_UNDEFINED;

		SingleUseCommandBuffer commandBuffer(device, &device->getQueues().getGraphicsQueue());
		commandBuffer.start();
		ImageBuffer::cmdTransitionImageLayout(commandBuffer.getCommandBuffer(), wantedLayout);
		commandBuffer.end();
	}

	if (properties.createImageView || properties.createSampler) {
		initImageView();

		descriptorImageInfo = {};
		descriptorImageInfo.imageView = imageView;
		descriptorImageInfo.imageLayout = properties.layout;
	}

	if (properties.createSampler) {
		initSampler();

		descriptorSamplerInfo = {};
		descriptorSamplerInfo.sampler = textureSampler;
		descriptorSamplerInfo.imageView = imageView;
		descriptorSamplerInfo.imageLayout = properties.layout;
	}
}

void ImageBuffer::init(VkImage image, bool createImageView) {
	this->properties.createImageView = createImageView;
	this->image = image;
	deleteImage = false;
	properties.layout = VK_IMAGE_LAYOUT_UNDEFINED;

	SingleUseCommandBuffer commandBuffer(device, &device->getQueues().getGraphicsQueue());
	commandBuffer.start();
	ImageBuffer::cmdTransitionImageLayout(commandBuffer.getCommandBuffer(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	commandBuffer.end();

	if (createImageView) {
		initImageView();

		descriptorImageInfo = {};
		descriptorImageInfo.imageView = imageView;
		descriptorImageInfo.imageLayout = properties.layout;
	}
}

void ImageBuffer::cmdCopyImage(VkCommandBuffer commandBuffer, ImageBuffer* destination) {
	VkImageLayout oldSrcLayout = (this->properties.layout == VK_IMAGE_LAYOUT_UNDEFINED ? VK_IMAGE_LAYOUT_GENERAL : this->properties.layout);
	VkImageLayout oldDstLayout = (destination->properties.layout == VK_IMAGE_LAYOUT_UNDEFINED ? VK_IMAGE_LAYOUT_GENERAL : destination->properties.layout);

	this->cmdTransitionImageLayout(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	destination->cmdTransitionImageLayout(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	VkImageCopy copyRegion{};
	copyRegion.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	copyRegion.srcOffset = { 0, 0, 0 };
	copyRegion.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	copyRegion.dstOffset = { 0, 0, 0 };
	copyRegion.extent = { properties.width, properties.height, 1 };
	vkCmdCopyImage(
		commandBuffer,
		this->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		destination->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1, &copyRegion
	);

	this->cmdTransitionImageLayout(commandBuffer, oldSrcLayout);
	destination->cmdTransitionImageLayout(commandBuffer, oldDstLayout);
}

void ImageBuffer::cmdClear(VkCommandBuffer commandBuffer) {
	VkClearColorValue clearColor;
	for (unsigned int i = 0; i < 4; ++i)
		clearColor.uint32[i] = 0;

	VkImageSubresourceRange subresourceRange{};
	subresourceRange.aspectMask = properties.aspectFlags;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = 1;
	subresourceRange.baseArrayLayer = 0;
	subresourceRange.layerCount = 1;

	vkCmdClearColorImage(commandBuffer, image, properties.layout, &clearColor, 1, &subresourceRange);
}

VkWriteDescriptorSet ImageBuffer::getWriteDescriptorSet(VkDescriptorSet descriptorSet, uint32_t binding) const {
	VkWriteDescriptorSet writeSet{};

	writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeSet.pNext = nullptr;
	writeSet.dstSet = descriptorSet;
	writeSet.dstBinding = binding;
	writeSet.descriptorCount = 1;
	writeSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	writeSet.pImageInfo = &descriptorImageInfo;

	return writeSet;
}

VkDescriptorType ImageBuffer::getDescriptorType() const {
	return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
}

VkWriteDescriptorSet ImageBuffer::getWriteDescriptorSetSampler(VkDescriptorSet descriptorSet, uint32_t binding) const {
	VkWriteDescriptorSet writeSet{};

	writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeSet.pNext = nullptr;
	writeSet.dstSet = descriptorSet;
	writeSet.dstBinding = binding;
	writeSet.descriptorCount = 1;
	writeSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeSet.pImageInfo = &descriptorSamplerInfo;

	return writeSet;
}

VkDescriptorType ImageBuffer::getDescriptorTypeSampler() const {
	return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
}

void ImageBuffer::initImageView() {
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = properties.format;
	viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.subresourceRange.aspectMask = properties.aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	if (vkCreateImageView(device->getDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
		throw InitException("vkCreateImageView", "failed to create image view!");
	}
}

void ImageBuffer::initSampler() {
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = device->getProperties().limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = properties.mipmapMode;

	if (vkCreateSampler(device->getDevice(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
		throw InitException("vkCreateSampler", "failed to create image texture sampler!");
	}
}

void ImageBuffer::cmdTransitionImageLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout) {
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = properties.layout;
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
		commandBuffer,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	properties.layout = newLayout;
}


const VkImage& ImageBuffer::getImage() const {
	return image;
}

const VkDeviceMemory& ImageBuffer::getMemory() const {
	return imageMemory;
}

const VkImageView& ImageBuffer::getImageView() const {
	return imageView;
}

void ImageBuffer::saveImageAsNetpbm(const std::string& filename) {
	size_t imageSize = properties.width * properties.height * 4;

	DataBuffer tmpBuffer(device);
	tmpBuffer.properties.bufferSize = imageSize;
	tmpBuffer.properties.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	tmpBuffer.properties.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	tmpBuffer.init();

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = {0, 0, 0};
	region.imageExtent = {
			properties.width,
			properties.height,
			1
	};

	SingleUseCommandBuffer cmd(device, &device->getQueues().getGraphicsQueue());
	cmd.start();
	this->cmdTransitionImageLayout(cmd.getCommandBuffer(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	vkCmdCopyImageToBuffer(cmd.getCommandBuffer(), image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, tmpBuffer.getBuffer(), 1, &region);
	this->cmdTransitionImageLayout(cmd.getCommandBuffer(), VK_IMAGE_LAYOUT_GENERAL);
	cmd.end();

	std::vector<char> data(imageSize);
	tmpBuffer.getData(data.data());

	std::ofstream file(filename, std::ios::out | std::ios::binary);
	unsigned int sizeformat = 255;
	file << "P6\n" << properties.width << "\n" << properties.height << "\n" << sizeformat << "\n";

	for (size_t i = 0; i < properties.width * properties.height; ++i) {
		file.write(&data[i * 4 + 2], 1);
		file.write(&data[i * 4 + 1], 1);
		file.write(&data[i * 4 + 0], 1);
	}

	file.close();
}
