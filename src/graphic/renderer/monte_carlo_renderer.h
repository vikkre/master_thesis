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


class MonteCarloRenderer: public Renderer {
	public:
		MonteCarloRenderer(Device* device);
		~MonteCarloRenderer();

		virtual void init() override;
		virtual void cmdRender(size_t index, VkCommandBuffer commandBuffer) override;
		virtual void updateUniforms(size_t index) override;

		std::vector<GraphicsObject*> objects;

		struct RenderSettings {
			Vector3f backgroundColor;
			Vector3f lightPosition;
			u_int32_t lightRayCount;
			u_int32_t lightJumpCount;
			u_int32_t visionJumpCount;
			float collectionDistance;
			u_int32_t visionRayPerPixelCount;
			float collectionDistanceShrinkFactor;
			u_int32_t lightCollectionCount;
			bool useCountLightCollecton;
		} renderSettings;

	private:
		void createTLAS();
		void createBuffers();
		void createDescriptorCollection();
		void createLightGenerationPipeline();
		void createKDPipeline();
		void createVisionPipeline();
		void createFinalRenderPipeline();

		Device* device;
		DescriptorCollection descriptorCollection;
		RayTracingPipeline lightGenerationPipeline;
		ComputePipeline kdPipeline;
		RayTracingPipeline visionPipeline;
		ComputePipeline finalRenderPipeline;
		std::vector<void*> objDataPtrs;

		SingleBufferDescriptor<TopAccelerationStructureBuffer> tlas;
		MultiBufferDescriptor<ImageBuffer> storageImagesRed;
		MultiBufferDescriptor<ImageBuffer> storageImagesGreen;
		MultiBufferDescriptor<ImageBuffer> storageImagesBlue;
		MultiBufferDescriptor<DataBuffer> globalDataBuffers;
		MultiBufferDescriptor<DataBuffer> renderSettingsBuffers;
		MultiBufferDescriptor<DataBuffer> countBuffers;
		MultiBufferDescriptor<DataBuffer> lightPointBuffers;
		MultiBufferDescriptor<DataBuffer> kdBuffers;
		MultiBufferDescriptor<DataBuffer> objDataBuffers;
};
