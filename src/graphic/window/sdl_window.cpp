#include "sdl_window.h"


const Uint32 SDLWindow::CREATE_WINDOW_FLAGS_DEFAULT = SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN;


SDLWindow::SDLWindow() {}

SDLWindow::~SDLWindow() {}

void SDLWindow::init(Device* device) {
	this->device = device;

	window = SDL_CreateWindow(
		windowName.c_str(),
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		windowSize[0], windowSize[1],
		crateWindowFlags
	);

	uint32_t extensionsCount = 0;
	SDL_Vulkan_GetInstanceExtensions(window, &extensionsCount, nullptr);
	requiredExtensions.resize(extensionsCount);
	SDL_Vulkan_GetInstanceExtensions(window, &extensionsCount, requiredExtensions.data());
}

void SDLWindow::createSurface() {
	if (!SDL_Vulkan_CreateSurface(window, device->getInstance(), &surface)) {
		throw InitException("SDL_Vulkan_CreateSurface", "failed to create window surface!");
	}
}

const VkSurfaceKHR& SDLWindow::getSurface() const {
	return surface;
}

const std::vector<const char*>& SDLWindow::getRequiredExtensions() const {
	return requiredExtensions;
}

VkExtent2D SDLWindow::getWindowExtend() const {
	int widht, height;
	SDL_Vulkan_GetDrawableSize(window, &widht, &height);
	return { (uint32_t) widht, (uint32_t) height };
}
