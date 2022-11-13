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


class UnidirectionalPathTracer: public Renderer {
	public:
		UnidirectionalPathTracer(Device* device);
		~UnidirectionalPathTracer();

		virtual void initRenderer() override;
		virtual void cmdRenderFrame(size_t index, VkCommandBuffer commandBuffer) override;
		virtual void updateRendererUniforms(size_t index) override;
		virtual void parseRendererInput(const InputEntry& inputEntry) override;

		struct RenderSettings {
			u_int32_t visionJumpCount;
			u_int32_t visionRayPerPixelCount;
		} renderSettings;

	private:
		void createBuffers();
		void createDescriptorCollection();
		void createVisionPipeline();
		void createFinalRenderPipeline();

		Device* device;
		DescriptorCollection descriptorCollection;
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
		RayTracingPipeline visionPipeline;
		ComputePipeline finalRenderPipeline;

		MultiBufferDescriptor<ImageBuffer> storageImagesRed;
		MultiBufferDescriptor<ImageBuffer> storageImagesGreen;
		MultiBufferDescriptor<ImageBuffer> storageImagesBlue;
		MultiBufferDescriptor<DataBuffer> renderSettingsBuffers;
};
