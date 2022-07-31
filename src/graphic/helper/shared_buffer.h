#pragma once

#include "buffer.h"
#include "data_buffer.h"


template<typename T>
class SharedBuffer: public Buffer {
	public:
		SharedBuffer(bool storage=false)
		:data(), buffer() {
			buffer.bufferSize(sizeof(T));
			buffer.usage = storage ? VK_BUFFER_USAGE_STORAGE_BUFFER_BIT : VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			buffer.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		}
		~SharedBuffer() {}

		void init() {
			buffer.init();
		}

		void updateBuffer() {
			buffer.passData((void*) &data);
		}

		void updateData() {
			buffer.getData((void*) &data);
		}

		virtual VkWriteDescriptorSet getWriteDescriptorSet(VkDescriptorSet descriptorSet, uint32_t binding) const override {
			return buffer.getWriteDescriptorSet(descriptorSet, binding);
		}

		virtual VkDescriptorType getDescriptorType() const override {
			return buffer.getDescriptorType();
		}

		T data;

	private:
		DataBuffer buffer;
};
