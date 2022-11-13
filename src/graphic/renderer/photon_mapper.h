#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include "../ray_tracing_pipeline.h"
#include "../compute_pipeline.h"

#include "../device.h"
#include "../helper/data_buffer.h"
#include "../helper/image_buffer.h"
#include "../graphics_object.h"
#include "../helper/single_buffer_descriptor.h"
#include "../helper/multi_buffer_descriptor.h"
#include "../helper/top_acceleration_structure_buffer.h"
#include "../helper/descriptor_collection.h"

#include "../../math/vector.h"

#include "renderer.h"


class PhotonMapper: public Renderer {
	public:
		PhotonMapper(Device* device);
		~PhotonMapper();

		virtual void initRenderer() override;
		virtual void cmdRenderFrame(size_t index, VkCommandBuffer commandBuffer) override;
		virtual void updateRendererUniforms(size_t index) override;
		virtual void parseRendererInput(const InputEntry& inputEntry) override;

		struct RenderSettings {
			u_int32_t lightRayCount;
			u_int32_t lightJumpCount;
			u_int32_t visionJumpCount;
			float collectionDistance;
			u_int32_t visionRayPerPixelCount;
			float collectionDistanceShrinkFactor;
			u_int32_t lightCollectionCount;
			u_int32_t useCountLightCollecton;
		} renderSettings;

	private:
		void createBuffers();
		void createDescriptorCollection();
		void createLightGenerationPipeline();
		void createKDPipeline();
		void createVisionPipeline();
		void createFinalRenderPipeline();

		Device* device;
		DescriptorCollection descriptorCollection;
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
		RayTracingPipeline lightGenerationPipeline;
		ComputePipeline kdPipeline;
		RayTracingPipeline visionPipeline;
		ComputePipeline finalRenderPipeline;

		MultiBufferDescriptor<ImageBuffer> storageImagesRed;
		MultiBufferDescriptor<ImageBuffer> storageImagesGreen;
		MultiBufferDescriptor<ImageBuffer> storageImagesBlue;
		MultiBufferDescriptor<DataBuffer> renderSettingsBuffers;
		MultiBufferDescriptor<DataBuffer> countBuffers;
		MultiBufferDescriptor<DataBuffer> lightPointBuffers;
		MultiBufferDescriptor<DataBuffer> kdBuffers;
};
