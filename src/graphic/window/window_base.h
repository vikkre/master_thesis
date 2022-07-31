#pragma once

#include <vulkan/vulkan.h>

#include <vector>


class Device;

class WindowBase {
	public:
		virtual ~WindowBase() {}

		virtual void init(Device* device)=0;

		virtual void createSurface()=0;
		virtual const VkSurfaceKHR& getSurface() const=0;
		virtual const std::vector<const char*>& getRequiredExtensions() const=0;
		virtual VkExtent2D getWindowExtend() const=0;
};
