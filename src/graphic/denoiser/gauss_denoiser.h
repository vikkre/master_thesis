#pragma once


#include "denoiser.h"

#include "../helper/data_buffer.h"
#include "../helper/descriptor_collection.h"
#include "../compute_pipeline.h"


class GaussDenoiser: public Denoiser {
	public:
		struct Settings {
			u_int32_t kernelSize;
			float sigma;
		} settings;

		GaussDenoiser(Device* device);
		~GaussDenoiser();

		virtual void initDenoiser() override;
		virtual void cmdRender(size_t index, VkCommandBuffer commandBuffer) override;
		virtual void updateUniforms(size_t index) override;
		virtual void parseInput(const InputEntry& inputEntry) override;
		
	private:
		void createBuffers();
		void createDescriptorCollection();
		void createDenoisePipeline();

		MultiBufferDescriptor<DataBuffer> settingsBuffers;
		DescriptorCollection descriptorCollection;
		ComputePipeline denoisePipeline;
};
