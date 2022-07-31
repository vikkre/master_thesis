#include "command_buffer.h"

#include "../device.h"
#include "queue.h"


SingleUseCommandBuffer::SingleUseCommandBuffer(const Device* device, const Queues::Queue* queue)
:device(device), queue(queue), commandBuffer(VK_NULL_HANDLE) {}

SingleUseCommandBuffer::~SingleUseCommandBuffer() {
	if (commandBuffer != VK_NULL_HANDLE) vkFreeCommandBuffers(device->getDevice(), queue->commandPool, 1, &commandBuffer);
}

void SingleUseCommandBuffer::start() {
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = queue->commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(device->getDevice(), &allocInfo, &commandBuffer) != VK_SUCCESS) {
		throw InitException("vkAllocateCommandBuffers", "could not allocate command buffer!");
	}

	VkCommandBufferBeginInfo commandBufferInfo{};
	commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	if (vkBeginCommandBuffer(commandBuffer, &commandBufferInfo) != VK_SUCCESS) {
		throw InitException("vkBeginCommandBuffer", "could not begin to record command buffer!");
	}
}

void SingleUseCommandBuffer::end(bool wait) {
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		throw InitException("vkEndCommandBuffer", "could not record command buffer!");
	}

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	
	if (vkQueueSubmit(queue->queue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
		throw InitException("vkQueueSubmit", "could not submit command buffer to queue!");
	}

	if (wait) this->wait();
}

void SingleUseCommandBuffer::wait() {
	if (vkQueueWaitIdle(queue->queue) != VK_SUCCESS) {
		throw InitException("vkQueueWaitIdle", "could not wait for command buffer queue!");
	}

	vkFreeCommandBuffers(device->getDevice(), queue->commandPool, 1, &commandBuffer);
	commandBuffer = VK_NULL_HANDLE;
}

const VkCommandBuffer& SingleUseCommandBuffer::getCommandBuffer() const {
	return commandBuffer;
}
