#pragma once

#include <vulkan/vulkan.h>

#include "../helper/top_acceleration_structure_buffer.h"
#include "../helper/descriptor_collection.h"
#include "../helper/single_buffer_descriptor.h"
#include "../helper/multi_buffer_descriptor.h"
#include "../graphics_object.h"

#include "../../input_parser.h"


class GraphicsObject;

class Renderer {
	public:
		struct RayTracingData {
			Matrix4f viewInverse;
			Matrix4f projInverse;
			Matrix4f view;
			Matrix4f proj;
			Vector3f backgroundColor;
		} rtData;

		Renderer(Device* device);
		virtual ~Renderer();

		void init();
		void cmdRender(size_t index, VkCommandBuffer commandBuffer);
		void updateUniforms(size_t index);
		void parseInput(const InputEntry& inputEntry);

		virtual void initRenderer()=0;
		virtual void cmdRenderFrame(size_t index, VkCommandBuffer commandBuffer)=0;
		virtual void updateRendererUniforms(size_t index)=0;
		virtual void parseRendererInput(const InputEntry& inputEntry)=0;

		void passObjects(const std::vector<GraphicsObject*>& objects);
		void setOutputImageBuffer(MultiBufferDescriptor<ImageBuffer>* outputImageBuffer);

		static void cmdPipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkCommandBuffer commandBuffer);

		static const std::string RMISS_SHADER;
		static const std::string RCHIT_SHADER;

	protected:
		std::vector<GraphicsObject*> objects;
		MultiBufferDescriptor<ImageBuffer>* outputImages;

		std::vector<const DescriptorCollection*> descriptors;
		void createPipelineLayout();
		VkPipelineLayout getPipelineLayout();

	private:
		void createTLAS();
		void createBuffers();
		void createDescriptorCollection();

		Device* device;
		DescriptorCollection descriptorCollection;
		std::vector<void*> objDataPtrs;

		MultiBufferDescriptor<TopAccelerationStructureBuffer> tlas;
		MultiBufferDescriptor<DataBuffer> rtDataBuffers;
		MultiBufferDescriptor<DataBuffer> objDataBuffers;

		VkPipelineLayout pipelineLayout;
};
