#include "multi_buffer_descriptor.h"


MultiBufferDescriptor::MultiBufferDescriptor(std::vector<Buffer*> buffers, uint32_t binding, VkShaderStageFlags shaderStageFlags)
:buffers(buffers), binding(binding), shaderStageFlags(shaderStageFlags) {}

MultiBufferDescriptor::~MultiBufferDescriptor() {}

void MultiBufferDescriptor::getWriteDescriptorSets(std::vector<VkWriteDescriptorSet>& writeDescriptorSets, const std::vector<VkDescriptorSet>& descriptorSets) const {
	if (descriptorSets.size() != buffers.size()) {
		throw InitException("MultiBufferDescriptor::getWriteDescriptorSets", "descriptorSets.size() and buffers.size() are unequal!");
	}
	
	for (unsigned int i = 0; i < buffers.size(); ++i) {
		writeDescriptorSets.push_back(buffers[i]->getWriteDescriptorSet(descriptorSets[i], binding));
	}
}

VkDescriptorSetLayoutBinding MultiBufferDescriptor::getLayoutBinding() const {
	if (buffers.size() == 0) {
		throw InitException("MultiBufferDescriptor::getLayoutBinding", "buffers.size() is 0!");
	}

	VkDescriptorSetLayoutBinding layoutBinding{};

	layoutBinding.binding = binding;
	layoutBinding.descriptorType = buffers[0]->getDescriptorType();
	layoutBinding.descriptorCount = 1;
	layoutBinding.stageFlags = shaderStageFlags;

	return layoutBinding;
}

const Buffer* MultiBufferDescriptor::getBuffer() const {
	return buffers[0];
}

