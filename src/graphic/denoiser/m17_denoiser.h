#pragma once


#include "denoiser.h"

#include "../helper/data_buffer.h"
#include "../helper/descriptor_collection.h"
#include "../compute_pipeline.h"


class M17Denoiser: public Denoiser {
	public:
		struct Settings {
			float historicalBlendingFactor;
			float smoothstepEdge0;
			float smoothstepEdge1;
		} settings;

		M17Denoiser(Device* device);
		~M17Denoiser();

		virtual void initDenoiser() override;
		virtual void cmdRender(size_t index, VkCommandBuffer commandBuffer) override;
		virtual void updateUniforms(size_t index) override;
		virtual void parseInput(const InputEntry& inputEntry) override;
		
	private:
		void createBuffers();
		void createDescriptorCollection();
		void createPipelines();

		DescriptorCollection descriptorCollection;
		ComputePipeline denoiseTemporalPipeline;
		ComputePipeline denoisePreBilateralPipeline;
		ComputePipeline denoiseFireflyPipeline;
		ComputePipeline denoiseDisocclusionPipeline;
		ComputePipeline denoisePostBilateralPipeline;

		MultiBufferDescriptor<DataBuffer> settingsBuffers;
		MultiBufferDescriptor<ImageBuffer> temporalImage;
		MultiBufferDescriptor<ImageBuffer> historicalBuffer;
		MultiBufferDescriptor<ImageBuffer> fireflyFreeImage;
		MultiBufferDescriptor<ImageBuffer> confidenceBuffer;
		MultiBufferDescriptor<ImageBuffer> kernelBuffer;
};
