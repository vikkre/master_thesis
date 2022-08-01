#pragma once

#include <vulkan/vulkan.h>

#include "device.h"
#include "swapchain.h"
#include "window/sdl_window.h"
#include "camera.h"
#include "pipeline/ray_tracing_pipeline.h"
#include "pipeline/ray_tracing_pipeline_points.h"
#include "helper/function_load.h"
#include "renderer/praktikums_render.h"


class GraphicsEngine {
	public:
		GraphicsEngine();
		~GraphicsEngine();

		void init();
		void initTlas();
		void render();

		Device device;
		Swapchain swapchain;
		PraktikumsRenderer renderer;
		// GraphicsPipeline pipeline;
		// RayTracingPipeline rtpipeline;
		// RayTracingPipelinePoints rtpipeline;

		bool commandBuffersRecorded;
};
