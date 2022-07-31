#pragma once

#include <vulkan/vulkan.h>

#include "queue.h"


class Device;

class SingleUseCommandBuffer {
	public:
		SingleUseCommandBuffer(const Device* device, const Queues::Queue* queue);
		~SingleUseCommandBuffer();

		void start();
		void end(bool wait=true);
		void wait();

		const VkCommandBuffer& getCommandBuffer() const;
	
	private:
		const Device* device;
		const Queues::Queue* queue;

		VkCommandBuffer commandBuffer;
};
