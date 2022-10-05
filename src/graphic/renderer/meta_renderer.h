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

// Result types:
// 0: Normal Vector
// 1: Position Vector (requires scaling)
// 2: Distance (requires scaling)
// 3: HandlingWeights


class MetaRenderer: public Renderer {
	public:
		MetaRenderer(Device* device);
		~MetaRenderer();

		virtual void initRenderer() override;
		virtual void cmdRenderFrame(size_t index, VkCommandBuffer commandBuffer) override;
		virtual void updateRendererUniforms(size_t index) override;
		virtual void parseRendererInput(const InputEntry& inputEntry) override;

		struct RenderSettings {
			uint32_t resultType;
			float scaling;
			uint32_t lightJump;
		} renderSettings;

	private:
		void createTLAS();
		void createBuffers();
		void createDescriptorCollection();
		void createPipeline();

		Device* device;
		DescriptorCollection descriptorCollection;
		RayTracingPipeline pipeline;

		std::vector<void*> objDataPtrs;

		SingleBufferDescriptor<TopAccelerationStructureBuffer> tlas;
		MultiBufferDescriptor<DataBuffer> objDataBuffers;
		MultiBufferDescriptor<DataBuffer> globalDataBuffers;
		MultiBufferDescriptor<DataBuffer> renderSettingsBuffers;
};
