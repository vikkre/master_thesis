#pragma once

#include <vulkan/vulkan.h>

#include "../../init_exception.h"


class Device;

class Image {
	public:
		Image(Device* device);
		~Image();

		void init();
		void init(VkImage image);
		void createImageView();
		void copyImage(const VkCommandBuffer* commandBuffer, Image* destination);

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

	private:
		void transitionImageLayout(const VkCommandBuffer* commandBuffer, VkImageLayout newLayout);

		Device* device;

		VkImage image;
		VkDeviceMemory imageMemory;
		VkImageView imageView;

		bool deleteImage;
};
