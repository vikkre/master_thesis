#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include "../ray_tracing_pipeline.h"

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

		void init();
		void cmdRender(size_t index, const VkCommandBuffer* commandBuffer);
		void updateUniforms(size_t index);

		std::vector<GraphicsObject*> objects;
		struct GlobalData {
			Matrix4f viewInverse;
			Matrix4f projInverse;
			Matrix4f view;
			Matrix4f proj;
			Vector3f backgroundColor;
			Vector3f lightPosition;
			u_int32_t lightJumpCount;
			u_int32_t visionJumpCount;
		} globalData;

	private:
		void createTLAS();
		void createBuffers();
		void createDescriptorCollection();
		void createLightGenerationPipeline();
		void createKDPipeline();

		Device* device;
		DescriptorCollection descriptorCollection;
		RayTracingPipeline lightGenerationPipeline;
		RayTracingPipeline kdPipeline;
		std::vector<void*> rtDataPtrs;

		SingleBufferDescriptor<TopAccelerationStructureBuffer> tlas;
		MultiBufferDescriptor<ImageBuffer> storageImages;
		MultiBufferDescriptor<DataBuffer> globalDataBuffers;
		MultiBufferDescriptor<DataBuffer> countBuffers;
		MultiBufferDescriptor<DataBuffer> lightPointBuffers;
		MultiBufferDescriptor<DataBuffer> kdBuffers;
		MultiBufferDescriptor<DataBuffer> rtDataBuffers;
};
