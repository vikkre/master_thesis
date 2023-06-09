#pragma once

#include <vulkan/vulkan.h>

#include "../helper/top_acceleration_structure_buffer.h"
#include "../helper/descriptor_collection.h"
#include "../helper/single_buffer_descriptor.h"
#include "../helper/multi_buffer_descriptor.h"
#include "../graphics_object.h"

#include "../../input_parser.h"
#include "../../mesh_manager.h"


class GraphicsObject;

class Renderer {
	public:
		struct RayTracingData {
			Matrix4f viewInverse;
			Matrix4f projInverse;
			Matrix4f view;
			Matrix4f proj;
			u_int32_t lightSourceCount;
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

		void passProbeData(const ProbeData& probeData);
		void passObjects(const std::vector<GraphicsObject*>& objects);
		void passLightSources(const std::vector<GraphicsObject*>& lightSources);
		void setOutputImageBuffer(MultiBufferDescriptor<ImageBuffer>* outputImageBuffer);

		static void cmdPipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkCommandBuffer commandBuffer);

		static const std::vector<std::string> RMISS_SHADERS;
		static const std::vector<std::string> RCHIT_SHADERS;

	protected:
		ProbeData probeData;
		std::vector<GraphicsObject*> objects;
		std::vector<GraphicsObject*> lightSources;
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
		std::vector<void*> lightSourceDataPtrs;

		MultiBufferDescriptor<TopAccelerationStructureBuffer> tlas;
		MultiBufferDescriptor<DataBuffer> rtDataBuffers;
		MultiBufferDescriptor<DataBuffer> objDataBuffers;
		MultiBufferDescriptor<DataBuffer> lightSourceDataBuffers;

		VkPipelineLayout pipelineLayout;
};
