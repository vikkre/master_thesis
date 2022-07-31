#include "single_buffer_descriptor.h"


SingleBufferDescriptor::SingleBufferDescriptor(Buffer* buffer, uint32_t binding, VkShaderStageFlags shaderStageFlags)
:buffer(buffer), binding(binding), shaderStageFlags(shaderStageFlags) {}

SingleBufferDescriptor::~SingleBufferDescriptor() {}

void SingleBufferDescriptor::getWriteDescriptorSets(std::vector<VkWriteDescriptorSet>& writeDescriptorSets, const std::vector<VkDescriptorSet>& descriptorSets) const {
	for (VkDescriptorSet descriptorSet: descriptorSets) {
		writeDescriptorSets.push_back(buffer->getWriteDescriptorSet(descriptorSet, binding));
	}
}

VkDescriptorSetLayoutBinding SingleBufferDescriptor::getLayoutBinding() const {
	VkDescriptorSetLayoutBinding layoutBinding{};

	layoutBinding.binding = binding;
	layoutBinding.descriptorType = buffer->getDescriptorType();
	layoutBinding.descriptorCount = 1;
	layoutBinding.stageFlags = shaderStageFlags;

	return layoutBinding;
}

const Buffer* SingleBufferDescriptor::getBuffer() const {
	return buffer;
}
