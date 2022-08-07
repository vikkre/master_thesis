#include "descriptor_collection.h"


#define GLOBAL_BINDING_SET_INDEX 0


DescriptorCollection::DescriptorCollection(Device* device)
:bufferDescriptors(), device(device),
descriptorSetLayout(VK_NULL_HANDLE), descriptorPool(VK_NULL_HANDLE), descriptorSets(), pipelineLayout(VK_NULL_HANDLE) {

}

DescriptorCollection::~DescriptorCollection() {
	vkDestroyPipelineLayout(device->getDevice(), pipelineLayout, nullptr);
	vkDestroyDescriptorPool(device->getDevice(), descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(device->getDevice(), descriptorSetLayout, nullptr);
}

void DescriptorCollection::init() {
	createDescriptorSetLayout();
	createDescriptorPool();
	createDescriptorSets();
	createPipelineLayout();
}

void DescriptorCollection::cmdBind(size_t index, const VkCommandBuffer* commandBuffer) const {
	vkCmdBindDescriptorSets(
		*commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
		pipelineLayout,
		GLOBAL_BINDING_SET_INDEX, 1, &descriptorSets.at(index),
		0, nullptr
	);

	vkCmdBindDescriptorSets(
		*commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
		pipelineLayout,
		GLOBAL_BINDING_SET_INDEX, 1, &descriptorSets.at(index),
		0, nullptr
	);
}

VkPipelineLayout DescriptorCollection::getPipelineLayout() const {
	return pipelineLayout;
}

void DescriptorCollection::createDescriptorSetLayout() {
	std::vector<VkDescriptorSetLayoutBinding> layoutBindings;

	for (unsigned int i = 0; i < bufferDescriptors.size(); ++i) {
		layoutBindings.push_back(bufferDescriptors[i]->getLayoutBinding(i));
	}
	
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = (uint32_t) layoutBindings.size();
	layoutInfo.pBindings = layoutBindings.data();

	if (vkCreateDescriptorSetLayout(device->getDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
		throw InitException("vkCreateDescriptorSetLayout", "failed to create descriptor set layout!");
	}
}

void DescriptorCollection::createDescriptorPool() {
	std::vector<VkDescriptorPoolSize> poolSizes;

	for (BufferDescriptor* bufferDescriptor: bufferDescriptors) {
		VkDescriptorPoolSize poolSize{};
		poolSize.type = bufferDescriptor->getBuffer()->getDescriptorType();
		poolSize.descriptorCount = device->renderInfo.swapchainImageCount;
		poolSizes.push_back(poolSize);
	}

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = poolSizes.size();
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = device->renderInfo.swapchainImageCount;

	if (vkCreateDescriptorPool(device->getDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
		throw InitException("vkCreateDescriptorPool", "failed to create descriptor pool!");
	}
}

void DescriptorCollection::createDescriptorSets() {
	descriptorSets.resize(device->renderInfo.swapchainImageCount);

	std::vector<VkDescriptorSetLayout> setLayouts;
	for (size_t i = 0; i < device->renderInfo.swapchainImageCount; ++i) {
		setLayouts.push_back(descriptorSetLayout);
	}

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = setLayouts.size();
	allocInfo.pSetLayouts = setLayouts.data();

	if (vkAllocateDescriptorSets(device->getDevice(), &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
		throw InitException("vkAllocateDescriptorSets", "failed to allocate rt pipeline descriptor sets!");
	}

	std::vector<VkWriteDescriptorSet> writeSets;

	for (unsigned int i = 0; i < bufferDescriptors.size(); ++i) {
		bufferDescriptors[i]->getWriteDescriptorSets(writeSets, descriptorSets, i);
	}

	vkUpdateDescriptorSets(device->getDevice(), writeSets.size(), writeSets.data(), 0, VK_NULL_HANDLE);
}

void DescriptorCollection::createPipelineLayout() {
	std::vector<VkDescriptorSetLayout> setLayouts {
		descriptorSetLayout,
	};

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = setLayouts.size();
	pipelineLayoutInfo.pSetLayouts = setLayouts.data();

	if (vkCreatePipelineLayout(device->getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw InitException("vkCreatePipelineLayout", "failed to create pipeline layout!");
	}
}
