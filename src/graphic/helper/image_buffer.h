#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <fstream>

#include "buffer.h"
#include "data_buffer.h"
#include "../../init_exception.h"


class Device;

class ImageBuffer: public Buffer {
	public:
		ImageBuffer(Device* device);
		~ImageBuffer();

		void init();
		void init(VkImage image, bool createImageView=false);
		void cmdCopyImage(VkCommandBuffer commandBuffer, ImageBuffer* destination);
		void cmdClear(VkCommandBuffer commandBuffer);
		virtual VkWriteDescriptorSet getWriteDescriptorSet(VkDescriptorSet descriptorSet, uint32_t binding) const override;
		virtual VkDescriptorType getDescriptorType() const override;

		const VkImage& getImage() const;
		const VkDeviceMemory& getMemory() const;
		const VkImageView& getImageView() const;

		void saveImageAsNetpbm(const std::string& filename);

		struct Properties {
			uint32_t width, height;
			VkFormat format;
			VkImageTiling tiling;
			VkImageUsageFlags usage;
			VkMemoryPropertyFlags properties;
			VkImageAspectFlags aspectFlags;
			VkImageLayout layout;
			bool createImageView;
		} properties;

	private:
		void initImageView();
		void cmdTransitionImageLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout);

		Device* device;

		VkImage image;
		VkDeviceMemory imageMemory;
		VkImageView imageView;
		VkDescriptorImageInfo descriptorImageInfo;

		bool deleteImage;
};
