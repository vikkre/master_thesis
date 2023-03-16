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


class Majercik2022BidirectionalPathTracer: public Renderer {
	public:
		Majercik2022BidirectionalPathTracer(Device* device);
		~Majercik2022BidirectionalPathTracer();

		virtual void initRenderer() override;
		virtual void cmdRenderFrame(size_t index, VkCommandBuffer commandBuffer) override;
		virtual void updateRendererUniforms(size_t index) override;
		virtual void parseRendererInput(const InputEntry& inputEntry) override;

		struct RenderSettings {
			u_int32_t visionJumpCount;
			u_int32_t lightJumpCount;
			u_int32_t maxDepth;

			float hysteresis;
			Vector3u probeCount;
			u_int32_t totalProbeCount;
			Vector3f probeStartCorner;
			Vector3f betweenProbeDistance;
			u_int32_t perProbeRayCount;
			float maxProbeRayDistance;
			u_int32_t probeSampleSideLength;
			float depthSharpness;
			float normalBias;
			u_int32_t linearBlending;
			float energyPreservation;

			u_int32_t candidateCount;
			u_int32_t sampleCount;

			u_int32_t probeCandidateCount;
			u_int32_t probeSampleCount;
			u_int32_t probeVisionJumpCount;
			u_int32_t probeLightJumpCount;
			u_int32_t probeMaxDepth;
		} renderSettings;

	private:
		void createBuffers();
		void createDescriptorCollection();
		void createProbeReservoirPipeline();
		void createProbePipeline();
		void createShadingUpdatePipeline();
		void createReservoirPipeline();
		void createFinalPipeline();

		Vector2u getIrradianceFieldSurfaceExtend() const;

		Device* device;
		DescriptorCollection descriptorCollection;
		RayTracingPipeline probeReservoirPipeline;
		RayTracingPipeline probePipeline;
		ComputePipeline shadingUpdatePipeline;
		RayTracingPipeline reservoirPipeline;
		RayTracingPipeline finalPipeline;

		MultiBufferDescriptor<DataBuffer> renderSettingsBuffers;
		MultiBufferDescriptor<DataBuffer> surfelBuffer;
		MultiBufferDescriptor<ImageBuffer> irradianceBuffer;
		MultiBufferDescriptor<ImageBuffer> depthBuffer;
		MultiSamplerDescriptor irradianceSampler;
		MultiSamplerDescriptor depthSampler;
		MultiBufferDescriptor<DataBuffer> rayPayloadsBuffers;
		MultiBufferDescriptor<DataBuffer> spatialReservoirsBuffers;
		MultiBufferDescriptor<DataBuffer> probeRayPayloadsBuffers;
		MultiBufferDescriptor<DataBuffer> probeSpatialReservoirsBuffers;
};
