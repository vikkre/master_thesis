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


class DDGIRenderer: public Renderer {
	public:
		DDGIRenderer(Device* device);
		~DDGIRenderer();

		virtual void init() override;
		virtual void cmdRender(size_t index, VkCommandBuffer commandBuffer) override;
		virtual void updateUniforms(size_t index) override;
		virtual void parseInput(const InputEntry& inputEntry) override;

		struct RenderSettings {
			Vector3f backgroundColor;
			Vector3f lightPosition;
			float betweenProbeDistance;
			u_int32_t singleDirectionProbeCount;
			u_int32_t totalProbeCount;
			u_int32_t perProbeRayCount;
			float maxProbeRayDistance;
			u_int32_t probeSampleSideLength;
			float depthSharpness;
		} renderSettings;

	private:
		void createTLAS();
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

		std::vector<void*> objDataPtrs;

		SingleBufferDescriptor<TopAccelerationStructureBuffer> tlas;
		MultiBufferDescriptor<DataBuffer> objDataBuffers;
		MultiBufferDescriptor<DataBuffer> globalDataBuffers;
		MultiBufferDescriptor<DataBuffer> renderSettingsBuffers;
		MultiBufferDescriptor<DataBuffer> surfelBuffer;
		MultiBufferDescriptor<ImageBuffer> irradianceBuffer;
		MultiBufferDescriptor<ImageBuffer> depthBuffer;
};
