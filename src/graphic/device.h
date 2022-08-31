#pragma once

#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>

#include <vector>
#include <set>

#include "../init_exception.h"
#include "window/window_base.h"
#include "helper/render_info.h"
#include "helper/queue.h"

class Device {
	public:
		Device(const std::string& basepath);
		~Device();

		void init();

		const VkInstance& getInstance() const;

		static const std::vector<const char*> VALIDATION_LAYERS_DEFAULT;
		static const std::vector<const char*> VALIDATION_LAYERS_EXTENDED;

		static const VkDebugUtilsMessageSeverityFlagsEXT VALIDATION_SEVERITY_DEFAULT;
		static const VkDebugUtilsMessageSeverityFlagsEXT VALIDATION_SEVERITY_ALL;

		static const std::vector<const char*> DEVICE_EXTENSIONS_DEFAULT;

		bool enableValidationLayers;
		std::vector<const char*> validationLayers;
		VkDebugUtilsMessageSeverityFlagsEXT severityFlags;
		std::vector<const char*> extensions;
		std::vector<const char*> deviceExtensions;

		const VkDevice& getDevice() const;
		const VkPhysicalDevice& getPhysicalDevice() const;
		const Queues& getQueues() const;

		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

		WindowBase* window;
		RenderInfo renderInfo;
		const std::string basepath;
	
	private:
		void createInstance();
		void setupDebugMessenger();
		void pickPhysicalDevice();
		void createLogicalDevice();

		bool checkValidationLayerSupport() const;
		void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) const;
		bool checkDeviceExtensionSupport(VkPhysicalDevice device);
		bool checkSwapChainSupport(VkPhysicalDevice device);
		bool isDeviceSuitable(VkPhysicalDevice device);

		PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;
		PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT;

		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
		VkPhysicalDevice physicalDevice;
		VkDevice device;
		Queues queues;
		VkPhysicalDeviceMemoryProperties memProperties;
};
