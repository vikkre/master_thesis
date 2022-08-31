#pragma once


#include "../device.h"

#include "../helper/multi_buffer_descriptor.h"
#include "../helper/image_buffer.h"
#include "../../input_parser.h"


class Denoiser {
	public:
		Denoiser(Device* device);
		virtual ~Denoiser();

		void init();
		virtual void initDenoiser();
		virtual void cmdRender(size_t index, VkCommandBuffer commandBuffer)=0;
		virtual void updateUniforms(size_t index)=0;
		virtual void parseInput(const InputEntry& inputEntry)=0;

		MultiBufferDescriptor<ImageBuffer>* getInputImageBuffer();
		void setOutputImageBuffer(MultiBufferDescriptor<ImageBuffer>* outputImageBuffer);

	protected:
		void createInputImages();

		Device* device;

		MultiBufferDescriptor<ImageBuffer> inputImages;
		MultiBufferDescriptor<ImageBuffer>* outputImages;
};
