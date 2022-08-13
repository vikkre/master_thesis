#include "graphics_object.h"

#include "mesh.h"
#include "device.h"

#define OBJECT_BINDING_SET_INDEX 1


GraphicsObject::GraphicsObject(const Device* device, const Mesh* mesh, const Vector3f& position)
:scale({1.0f, 1.0f, 1.0f}), rotation(), position(position),
color(Vector3f({1.0f, 1.0f, 1.0f})),
diffuseWeight(1.0f), reflectWeight(0.0f), glossyWeight(0.0f), transparentWeight(0.0f),
mesh(mesh), device(device) {}

GraphicsObject::~GraphicsObject() {
	vkDeviceWaitIdle(device->getDevice());
}

void GraphicsObject::passBufferData(size_t /* index */) {
	rtData.objectMatrix = getMatrix();
	rtData.color = color;

	rtData.vertexAddress = mesh->getVertexBuffer().getAddress();
	rtData.indexAddress = mesh->getIndexBuffer().getAddress();

	float totalWeight = diffuseWeight + reflectWeight + glossyWeight + transparentWeight;

	rtData.diffuseThreshold = diffuseWeight / totalWeight;
	rtData.reflectThreshold = reflectWeight / totalWeight + rtData.diffuseThreshold;
	rtData.glossyThreshold = glossyWeight / totalWeight + rtData.reflectThreshold;
	rtData.transparentThreshold = transparentWeight / totalWeight + rtData.glossyThreshold;
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

size_t GraphicsObject::getRTDataSize() {
	return sizeof(GraphicsObject::RTData);
}
