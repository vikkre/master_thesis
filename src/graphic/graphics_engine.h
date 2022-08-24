#pragma once

#include <vulkan/vulkan.h>

#include "device.h"
#include "swapchain.h"
#include "window/sdl_window.h"
#include "camera.h"
#include "helper/function_load.h"
#include "renderer/renderer.h"
#include "denoiser/denoiser.h"
// #include "renderer/praktikums_renderer.h"
// #include "renderer/monte_carlo_renderer.h"


class GraphicsEngine {
	public:
		GraphicsEngine();
		~GraphicsEngine();

		void init();
		void initTlas();
		void render();

		Device device;
		Swapchain swapchain;
		Renderer* renderer;
		std::vector<Denoiser*> denoisers;

		bool commandBuffersRecorded;
};
