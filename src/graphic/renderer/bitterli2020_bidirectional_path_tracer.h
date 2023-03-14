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


class Bitterli2020BidirectionalPathTracer: public Renderer {
	public:
		Bitterli2020BidirectionalPathTracer(Device* device);
		~Bitterli2020BidirectionalPathTracer();

		virtual void initRenderer() override;
		virtual void cmdRenderFrame(size_t index, VkCommandBuffer commandBuffer) override;
		virtual void updateRendererUniforms(size_t index) override;
		virtual void parseRendererInput(const InputEntry& inputEntry) override;

		struct RenderSettings {
			u_int32_t visionJumpCount;
			u_int32_t lightJumpCount;
			u_int32_t maxDepth;
			u_int32_t candidateCount;
			u_int32_t sampleCount;
		} renderSettings;

	private:
		void createBuffers();
		void createDescriptorCollection();
		void createReservoirPipeline();
		void createResultPipeline();

		Device* device;
		DescriptorCollection descriptorCollection;
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
		RayTracingPipeline reservoirPipeline;
		RayTracingPipeline resultPipeline;

		MultiBufferDescriptor<DataBuffer> renderSettingsBuffers;
		MultiBufferDescriptor<DataBuffer> rayPayloadsBuffers;
		MultiBufferDescriptor<DataBuffer> spatialReservoirsBuffers;
};
