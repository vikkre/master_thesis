#include "queue.h"

#include "../device.h"


Queues::Queues(Device* device):
device(device) {}

Queues::~Queues() {
	// vkDestroyCommandPool(device->getDevice(), graphicsQueue.commandPool, nullptr);
	// vkDestroyCommandPool(device->getDevice(), transferQueue.commandPool, nullptr);
	// vkDestroyCommandPool(device->getDevice(), computeQueue.commandPool, nullptr);
}

void Queues::findQueueFamilies(VkPhysicalDevice physicalDevice) {
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	for (size_t i = 0; i < queueFamilies.size(); ++i) {
		const VkQueueFamilyProperties& queueFamily = queueFamilies.at(i);

		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			graphicsQueue.present = true;
			graphicsQueue.queueIndex = i;

		} else if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) {
			computeQueue.present = true;
			computeQueue.queueIndex = i;
			
		} else if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) {
			transferQueue.present = true;
			transferQueue.queueIndex = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, device->window->getSurface(), &presentSupport);

		if (presentSupport) {
			presentQueue.present = true;
			presentQueue.queueIndex = i;
		}
	}

	if (!transferQueue.present && graphicsQueue.present) {
		transferQueue.present = true;
		transferQueue.queueIndex = graphicsQueue.queueIndex;
	}
}

void Queues::Queue::init(VkDevice device, bool withCommandPool, VkCommandPoolCreateFlags commandPoolFlags) {
	vkGetDeviceQueue(device, queueIndex, 0, &queue);

	if (withCommandPool) {
		VkCommandPoolCreateInfo commandPoolInfo{};
		commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolInfo.queueFamilyIndex = queueIndex;
		commandPoolInfo.flags = commandPoolFlags;

		if (vkCreateCommandPool(device, &commandPoolInfo, nullptr, &commandPool) != VK_SUCCESS) {
			throw InitException("vkCreateCommandPool", "failed to create command pool!");
		}
	}
}

void Queues::init() {
	graphicsQueue.init(device->getDevice());
	transferQueue.init(device->getDevice(), true, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
	presentQueue.init(device->getDevice(), false);
	computeQueue.init(device->getDevice());
}

void Queues::reset() {
	vkDestroyCommandPool(device->getDevice(), graphicsQueue.commandPool, nullptr);
	vkDestroyCommandPool(device->getDevice(), transferQueue.commandPool, nullptr);
	vkDestroyCommandPool(device->getDevice(), computeQueue.commandPool, nullptr);
}

bool Queues::allQueuesPresent() const {
	return graphicsQueue.present && transferQueue.present && presentQueue.present;
}

const Queues::Queue& Queues::getGraphicsQueue() const {
	return graphicsQueue;
}

const Queues::Queue& Queues::getTransferQueue() const {
	return transferQueue;
}

const Queues::Queue& Queues::getPresentQueue() const {
	return presentQueue;
}

const Queues::Queue& Queues::getComputeQueue() const {
	return computeQueue;
}
