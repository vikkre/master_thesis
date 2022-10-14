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


class Majercik2019: public Renderer {
	public:
		Majercik2019(Device* device);
		~Majercik2019();

		virtual void initRenderer() override;
		virtual void cmdRenderFrame(size_t index, VkCommandBuffer commandBuffer) override;
		virtual void updateRendererUniforms(size_t index) override;
		virtual void parseRendererInput(const InputEntry& inputEntry) override;

		struct RenderSettings {
			Vector3f lightPosition;
			u_int32_t lightJumpCount;
			u_int32_t visionJumpCount;
			float betweenProbeDistance;
			u_int32_t singleDirectionProbeCount;
			u_int32_t totalProbeCount;
			u_int32_t perProbeRayCount;
			float maxProbeRayDistance;
			u_int32_t probeSampleSideLength;
			float depthSharpness;
			float normalBias;
			float crushThreshold;
			u_int32_t linearBlending;
			float energyPreservation;
			float texelGetProbeDirectionFactor;
			float texelGetNormalFactor;
		} renderSettings;

	private:
		void createBuffers();
		void createDescriptorCollection();
		void createProbePipeline();
		void createShadingUpdatePipeline();
		void createFinalPipeline();

		Vector2u getIrradianceFieldSurfaceExtend() const;

		Device* device;
		DescriptorCollection descriptorCollection;
		RayTracingPipeline probePipeline;
		ComputePipeline shadingUpdatePipeline;
		RayTracingPipeline finalPipeline;

		MultiBufferDescriptor<DataBuffer> renderSettingsBuffers;
		MultiBufferDescriptor<DataBuffer> surfelBuffer;
		MultiBufferDescriptor<ImageBuffer> irradianceBuffer;
		MultiBufferDescriptor<ImageBuffer> depthBuffer;
		MultiSamplerDescriptor irradianceSampler;
		MultiSamplerDescriptor depthSampler;
};
