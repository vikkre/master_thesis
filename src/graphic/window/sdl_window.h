#pragma once

#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include <vector>
#include <string>

#include "../../math/vector.h"
#include "../../init_exception.h"

#include "window_base.h"
#include "../device.h"


class SDLWindow: public WindowBase {
	public:
		SDLWindow();
		~SDLWindow();

		virtual void init(Device* device) override;

		virtual void createSurface() override;
		virtual const VkSurfaceKHR& getSurface() const override;
		virtual const std::vector<const char*>& getRequiredExtensions() const override;
		virtual VkExtent2D getWindowExtend() const override;

		std::string windowName;
		Vector2i windowSize;
		Uint32 crateWindowFlags;

		static const Uint32 CREATE_WINDOW_FLAGS_DEFAULT;
	private:
		Device* device;
		SDL_Window* window;

		std::vector<const char*> requiredExtensions;
		VkSurfaceKHR surface;
};
