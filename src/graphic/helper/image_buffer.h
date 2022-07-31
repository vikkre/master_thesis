#pragma once

#include <vulkan/vulkan.h>

#include "buffer.h"
#include "../../init_exception.h"


class Device;

class ImageBuffer: public Buffer {
	public:
		ImageBuffer(Device* device);
		~ImageBuffer();

		void init();
		void init(VkImage image, bool createImageView=false);
		void cmdCopyImage(const VkCommandBuffer* commandBuffer, ImageBuffer* destination);
		virtual VkWriteDescriptorSet getWriteDescriptorSet(VkDescriptorSet descriptorSet, uint32_t binding) const override;
		virtual VkDescriptorType getDescriptorType() const override;

		const VkImage& getImage() const;
		const VkDeviceMemory& getMemory() const;
		const VkImageView& getImageView() const;

		uint32_t width, height;
		VkFormat format;
		VkImageTiling tiling;
		VkImageUsageFlags usage;
		VkMemoryPropertyFlags properties;
		VkImageAspectFlags aspectFlags;
		VkImageLayout layout;
		bool createImageView;

	private:
		void initImageView();
		void cmdTransitionImageLayout(const VkCommandBuffer* commandBuffer, VkImageLayout newLayout);

		Device* device;

		VkImage image;
		VkDeviceMemory imageMemory;
		VkImageView imageView;
		VkDescriptorImageInfo descriptorImageInfo;

		bool deleteImage;
};
