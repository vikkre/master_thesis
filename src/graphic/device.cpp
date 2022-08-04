#include "device.h"


const std::vector<const char*> Device::VALIDATION_LAYERS_DEFAULT = {
	"VK_LAYER_KHRONOS_validation",
};

const std::vector<const char*> Device::VALIDATION_LAYERS_EXTENDED = {
	"VK_LAYER_KHRONOS_validation",
	"VK_LAYER_LUNARG_api_dump",
};

const VkDebugUtilsMessageSeverityFlagsEXT Device::VALIDATION_SEVERITY_DEFAULT =
VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

const VkDebugUtilsMessageSeverityFlagsEXT Device::VALIDATION_SEVERITY_ALL =
VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

const std::vector<const char*> Device::DEVICE_EXTENSIONS_DEFAULT = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
	VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
	VK_KHR_RAY_QUERY_EXTENSION_NAME,
	VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
};


VKAPI_ATTR VkBool32 VKAPI_CALL validationLayerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void*) {
	SDL_LogError(SDL_LOG_CATEGORY_RENDER, "validation layer: %s\n", pCallbackData->pMessage);

	return VK_FALSE;
}


Device::Device()
:enableValidationLayers(false),
validationLayers(), severityFlags(), extensions(),
vkCreateDebugUtilsMessengerEXT(VK_NULL_HANDLE), vkDestroyDebugUtilsMessengerEXT(VK_NULL_HANDLE),
instance(VK_NULL_HANDLE), debugMessenger(VK_NULL_HANDLE), queues(this) {}

Device::~Device() {
	queues.reset();
	if (enableValidationLayers) {
		vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	}
	vkDestroyDevice(device, nullptr);
	vkDestroySurfaceKHR(instance, window->getSurface(), nullptr);
	vkDestroyInstance(instance, nullptr);
}

void Device::init() {
	window->init(this);

	extensions = window->getRequiredExtensions();
	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	createInstance();

	window->createSurface();

	vkCreateDebugUtilsMessengerEXT  = (PFN_vkCreateDebugUtilsMessengerEXT)  vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

	setupDebugMessenger();
	pickPhysicalDevice();
	createLogicalDevice();
	queues.init();
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
}

const VkInstance& Device::getInstance() const {
	return instance;
}

void Device::createInstance() {
	if (enableValidationLayers && !checkValidationLayerSupport()) {
		throw InitException("enableValidationLayers", "validation layers requested, but not available!");
	}

	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Ray Tracing Test";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 2, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 2, 0);
	appInfo.apiVersion = VK_API_VERSION_1_2;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = (uint32_t) extensions.size();
	createInfo.ppEnabledExtensionNames = extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

	if (enableValidationLayers) {
		createInfo.enabledLayerCount = (uint32_t) validationLayers.size();
		createInfo.ppEnabledLayerNames = validationLayers.data();

		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
	} else {
		createInfo.enabledLayerCount = 0;

		createInfo.pNext = nullptr;
	}

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
		throw InitException("vkCreateInstance", "failed to create instance!");
	}
}

void Device::setupDebugMessenger() {
	if (!enableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(createInfo);

	if (vkCreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
		throw InitException("vkCreateDebugUtilsMessengerEXT", "failed to set up debug messenger!");
	}
}

bool Device::checkValidationLayerSupport() const {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName: validationLayers) {
		bool layerFound = false;

		for (const VkLayerProperties& layerProperties: availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}

void Device::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) const {
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = severityFlags;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = validationLayerCallback;
}

void Device::pickPhysicalDevice() {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		throw InitException("pickPhysicalDevice", "failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	for (const VkPhysicalDevice& device: devices) {
		if (isDeviceSuitable(device)) {
			physicalDevice = device;
			break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE) {
		throw InitException("pickPhysicalDevice", "failed to find a suitable GPU!");
	}
}

void Device::createLogicalDevice() {
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = {
		queues.getGraphicsQueue().queueIndex,
		queues.getTransferQueue().queueIndex,
		queues.getPresentQueue().queueIndex
	};

	float queuePriority = 1.0f;
	for (uint32_t queueFamily: uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures2 deviceFeatures10{};
	deviceFeatures10.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	deviceFeatures10.features.shaderInt64 = VK_TRUE;
	deviceFeatures10.features.sparseResidencyBuffer = VK_TRUE;
	deviceFeatures10.features.sparseResidencyAliased = VK_TRUE;
	deviceFeatures10.features.sparseBinding = VK_TRUE;

	VkPhysicalDeviceVulkan11Features deviceFeatures11{};
	deviceFeatures11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
	deviceFeatures11.pNext = &deviceFeatures10;

	VkPhysicalDeviceVulkan12Features deviceFeatures12{};
	deviceFeatures12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	deviceFeatures12.pNext = &deviceFeatures11;
	deviceFeatures12.bufferDeviceAddress = VK_TRUE;
	deviceFeatures12.bufferDeviceAddressMultiDevice = VK_TRUE;
	deviceFeatures12.runtimeDescriptorArray = VK_TRUE;
	deviceFeatures12.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;

	VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures{};
	rayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
	rayTracingPipelineFeatures.rayTracingPipeline = VK_TRUE;
	rayTracingPipelineFeatures.pNext = &deviceFeatures12;

	VkPhysicalDeviceAccelerationStructureFeaturesKHR accelearationStructureFeatures{};
	accelearationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
	accelearationStructureFeatures.accelerationStructure = VK_TRUE;
	accelearationStructureFeatures.pNext = &rayTracingPipelineFeatures;

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pNext = &accelearationStructureFeatures;
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = (uint32_t) queueCreateInfos.size();
	createInfo.enabledExtensionCount = (uint32_t) deviceExtensions.size();
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();
	createInfo.enabledLayerCount = 0;

	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
		throw InitException("vkCreateDevice", "failed to create logical device!");
	}
}

bool Device::checkDeviceExtensionSupport(VkPhysicalDevice device) {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const VkExtensionProperties& extension: availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

bool Device::checkSwapChainSupport(VkPhysicalDevice device) {
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, window->getSurface(), &renderInfo.surfaceCapabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, window->getSurface(), &formatCount, nullptr);
	if (formatCount > 0) {
		renderInfo.surfaceFormats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, window->getSurface(), &formatCount, renderInfo.surfaceFormats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, window->getSurface(), &presentModeCount, nullptr);
	if (presentModeCount > 0) {
		renderInfo.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, window->getSurface(), &presentModeCount, renderInfo.presentModes.data());
	}

	return formatCount >= 1 && presentModeCount >= 1;
}

bool Device::isDeviceSuitable(VkPhysicalDevice device) {
	queues.findQueueFamilies(device);

	bool extensionsSupported = checkDeviceExtensionSupport(device);
	bool swapChainAdequate = checkSwapChainSupport(device);

	return queues.allQueuesPresent() && extensionsSupported && swapChainAdequate;
}

const VkDevice& Device::getDevice() const {
	return device;
}

const VkPhysicalDevice& Device::getPhysicalDevice() const {
	return physicalDevice;
}

const Queues& Device::getQueues() const {
	return queues;
}

uint32_t Device::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const {
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw InitException("Device::findMemoryType", "Failed to find suitable memory type!");
}
