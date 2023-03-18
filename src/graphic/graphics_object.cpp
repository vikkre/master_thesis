#include "graphics_object.h"

#include "mesh.h"
#include "device.h"

#define OBJECT_BINDING_SET_INDEX 1


GraphicsObject::GraphicsObject(const Device* device, const Mesh* mesh, const Vector3f& position)
:scale({1.0f, 1.0f, 1.0f}), rotation(), position(position),
color(Vector3f({1.0f, 1.0f, 1.0f})),
diffuseWeight(1.0f), reflectWeight(0.0f), transparentWeight(0.0f),
move(false), rotate(false), weightDone(false),
moveStartPos(position), moveStopPos(position), moveSpeed(0.0f), movedDistance(-0.5f),
rotationAxis({0.0f, 0.0f, 0.0f}), totalRotation(), rotationSpeed(0.0f), rotatedAngle(0.0f),
mesh(mesh), device(device) {}

GraphicsObject::~GraphicsObject() {
	vkDeviceWaitIdle(device->getDevice());
}

void GraphicsObject::update(float deltaTime) {
	if (move) {
		movedDistance += moveSpeed * deltaTime;
		float t = sin(movedDistance * M_PI) * 0.5f + 0.5f;
		position = lerp(moveStartPos, moveStopPos, t);
	}

	if (rotate) {
		rotatedAngle += rotationSpeed * deltaTime;
		totalRotation = Rotation(rotationAxis, rotatedAngle).apply(rotation);
	} else {
		totalRotation = rotation;
	}
}

void GraphicsObject::passBufferData(size_t /* index */) {
	rtData.objectMatrix = getMatrix();
	rtData.color = color;

	rtData.vertexAddress = mesh->getVertexBuffer().getAddress();
	rtData.indexAddress = mesh->getIndexBuffer().getAddress();

	float totalWeight = diffuseWeight + reflectWeight + transparentWeight;

	rtData.diffuseThreshold = diffuseWeight / totalWeight;
	rtData.reflectThreshold = reflectWeight / totalWeight + rtData.diffuseThreshold;
	rtData.transparentThreshold = transparentWeight / totalWeight + rtData.reflectThreshold;

	rtData.indexCount = mesh->indices.size() / 3;


	if (rtData.lightSource == 1 && !weightDone) {
		Matrix4f tm = rtData.objectMatrix;
		tm.transpose();
		std::vector<Vector3f> tPositions(mesh->vertices.size());
		for (size_t i = 0; i < mesh->vertices.size(); ++i) {
			Vector3f pos = mesh->vertices[i].point;
			Vector4f hpos = Vector4f({pos[0], pos[1], pos[2], 1.0f});
			Vector4f tpos = tm * hpos;
			tPositions[i] = Vector3f({tpos[0], tpos[1], tpos[2]});
		}

		float totalArea = 0.0f;
		for (size_t i = 0; i < mesh->indices.size(); i += 3) {
			Vector3f& v1 = tPositions[mesh->indices[i + 0]];
			Vector3f& v2 = tPositions[mesh->indices[i + 1]];
			Vector3f& v3 = tPositions[mesh->indices[i + 2]];
			totalArea += getTriangleArea(v1, v2, v3);
		}
		
		weightDone = true;
	}
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

	objectMatrix *= getScaleMatrix(scale);
	objectMatrix *= totalRotation.getMatrix();
	objectMatrix *= getTranslationMatrix(position);

	return objectMatrix;
}

size_t GraphicsObject::getRTDataSize() {
	return sizeof(GraphicsObject::RTData);
}
