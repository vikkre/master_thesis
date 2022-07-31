#pragma once

#include <vulkan/vulkan.h>


class Device;

class Queues {
	public:
		struct Queue {
			bool present;
			uint32_t queueIndex;
			VkQueue queue;
			VkCommandPool commandPool;

			void init(VkDevice device, bool withCommandPool=true, VkCommandPoolCreateFlags commandPoolFlags=0);
		};

		Queues(Device* device);
		~Queues();

		void findQueueFamilies(VkPhysicalDevice physicalDevice);
		void init();
		void reset();

		bool allQueuesPresent() const;

		const Queue& getGraphicsQueue() const;
		const Queue& getTransferQueue() const;
		const Queue& getPresentQueue() const;
		const Queue& getComputeQueue() const;
	
	private:
		Device* device;

		Queue graphicsQueue;
		Queue transferQueue;
		Queue presentQueue;
		Queue computeQueue;
};
