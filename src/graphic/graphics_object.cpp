#include "graphics_object.h"

#include "mesh.h"
#include "device.h"
#include "helper/descriptor.h"

#define OBJECT_BINDING_SET_INDEX 1


GraphicsObject::GraphicsObject(const Device* device, const Mesh* mesh, const Vector3f& position)
:scale({1.0f, 1.0f, 1.0f}), rotation(), position(position),
objectData(), mesh(mesh), device(device),
descriptorPool(VK_NULL_HANDLE), descriptors() {
	objectData.color = Vector3f({1.0f, 1.0f, 1.0f});
	rtData.reflect = 0.0f;
}

GraphicsObject::~GraphicsObject() {
	vkDeviceWaitIdle(device->getDevice());

	if (descriptorPool != VK_NULL_HANDLE) {
		vkDestroyDescriptorPool(device->getDevice(), descriptorPool, nullptr);
	}
}

void GraphicsObject::init() {
	createDescriptorPool();
	createDescriptors();
}

void GraphicsObject::passBufferData(size_t /* index */) {
	objectData.objectMatrix = getMatrix();

	// descriptors.at(index).getBuffer().passData((void*) &objectData);

	rtData.objectMatrix = getMatrix();
	rtData.color = objectData.color;
	rtData.vertexAddress = mesh->getVertexBuffer().getAddress();
	rtData.indexAddress = mesh->getIndexBuffer().getAddress();
}

void GraphicsObject::recordCommandBuffer(size_t index, const VkCommandBuffer* commandBuffer) {
	descriptors.at(index).bind(commandBuffer);

	mesh->recordCommandBuffer(commandBuffer);
}

GraphicsObject::ObjectInfo GraphicsObject::getObjectInfo() const {
	GraphicsObject::ObjectInfo info;
	info.dataPtr = (void*) &rtData;

	Matrix4f objectMatrix = getMatrix();
	VkTransformMatrixKHR transformMatrix;

	for (size_t i = 0; i < 4; ++i) {
		for (size_t j = 0; j < 3; ++j) {
			transformMatrix.matrix[j][i] = objectMatrix[i][j];
		}
	}
	
	info.instance = {};
	info.instance.transform = transformMatrix;
	info.instance.instanceCustomIndex = 0;
	info.instance.mask = 0xFF;
	info.instance.instanceShaderBindingTableRecordOffset = 0;
	info.instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
	info.instance.accelerationStructureReference = mesh->getBlas().getDeviceAddress();

	return info;
}

Matrix4f GraphicsObject::getMatrix() const {
	Matrix4f objectMatrix;

	objectMatrix *= rotation.getMatrix();
	objectMatrix *= getScaleMatrix(scale);
	objectMatrix *= getTranslationMatrix(position);

	return objectMatrix;
}

void GraphicsObject::createDescriptorPool() {
	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = device->renderInfo.swapchainImageCount;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = device->renderInfo.swapchainImageCount;

	if (vkCreateDescriptorPool(device->getDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
		throw InitException("vkCreateDescriptorPool", "failed to create descriptor pool!");
	}
}

void GraphicsObject::createDescriptors() {
	descriptors.reserve(device->renderInfo.swapchainImageCount);

	for (size_t i = 0; i < device->renderInfo.swapchainImageCount; ++i) {
		descriptors.emplace_back(device);

		descriptors.at(i).bindingSetIndex = OBJECT_BINDING_SET_INDEX;
		descriptors.at(i).bufferSize = sizeof(GraphicsObject::ObjectData);
		descriptors.at(i).descriptorPool = descriptorPool;
		descriptors.at(i).setLayout = device->renderInfo.objectDescriptorSetLayout;

		descriptors.at(i).init();
	}
}

std::vector<VkDescriptorSetLayoutBinding> GraphicsObject::getUniformBindings() {
	std::vector<VkDescriptorSetLayoutBinding> bindings(1);

	bindings.at(0).binding         = 0;
	bindings.at(0).descriptorCount = 1;
	bindings.at(0).descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bindings.at(0).stageFlags      = VK_SHADER_STAGE_VERTEX_BIT;

	return bindings;
}

uint32_t GraphicsObject::getBindingSet() {
	return OBJECT_BINDING_SET_INDEX;
}

size_t GraphicsObject::getRTDataSize() {
	return sizeof(GraphicsObject::RTData);
}
