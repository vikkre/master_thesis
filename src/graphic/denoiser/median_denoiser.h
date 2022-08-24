#pragma once


#include "denoiser.h"

#include "../helper/data_buffer.h"
#include "../helper/descriptor_collection.h"
#include "../compute_pipeline.h"


class MedianDenoiser: public Denoiser {
	public:
		struct Settings {
			u_int32_t kernelSize;
		} settings;

		MedianDenoiser(Device* device);
		~MedianDenoiser();

		virtual void initDenoiser() override;
		virtual void cmdRender(size_t index, VkCommandBuffer commandBuffer) override;
		virtual void updateUniforms(size_t index) override;
	private:
		void createBuffers();
		void createDescriptorCollection();
		void createDenoisePipeline();

		MultiBufferDescriptor<DataBuffer> settingsBuffers;
		DescriptorCollection descriptorCollection;
		ComputePipeline denoisePipeline;
};
