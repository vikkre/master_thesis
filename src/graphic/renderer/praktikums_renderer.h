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

#include "renderer.h"


class PraktikumsRenderer: public Renderer {
	public:
		PraktikumsRenderer(Device* device);
		~PraktikumsRenderer();

		virtual void init() override;
		virtual void cmdRender(size_t index, VkCommandBuffer commandBuffer) override;
		virtual void updateUniforms(size_t index) override;
		virtual void passObjects(const std::vector<GraphicsObject*>& objects) override;
		virtual void parseInput(const InputEntry& inputEntry) override;

		struct GlobalData {
			Matrix4f viewInverse;
			Matrix4f projInverse;
			Matrix4f view;
			Matrix4f proj;
			Vector3f backgroundColor;
			Vector3f lightPosition;
		} globalData;

	private:
		void createTLAS();
		void createBuffers();
		void createDescriptorCollection();
		void createPipeline();

		Device* device;
		DescriptorCollection descriptorCollection;
		RayTracingPipeline pipeline;

		std::vector<GraphicsObject*> objects;
		std::vector<void*> objDataPtrs;

		SingleBufferDescriptor<TopAccelerationStructureBuffer> tlas;
		MultiBufferDescriptor<DataBuffer> globalDataBuffers;
		MultiBufferDescriptor<DataBuffer> objDataBuffers;
};
