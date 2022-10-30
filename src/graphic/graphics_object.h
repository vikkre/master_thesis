#pragma once

#include <vulkan/vulkan.h>

#include "../math/vector.h"
#include "../math/matrix.h"
#include "../math/rotation.h"

#include <vector>


class Mesh;
class Device;

class GraphicsObject {
	public:
		struct RTData {
			Matrix4f objectMatrix;
			Vector3f color;
			uint64_t vertexAddress;
			uint64_t indexAddress;
			float diffuseThreshold;
			float reflectThreshold;
			float transparentThreshold;
			float refractionIndex;
			u_int32_t lightSource;
			u_int32_t indexCount;
		};

		struct ObjectInfo {
			VkAccelerationStructureInstanceKHR instance;
			void* dataPtr;
		};

		GraphicsObject(const Device* device, const Mesh* mesh, const Vector3f& position);
		~GraphicsObject();

		void init();
		void update(float deltaTime);
		void passBufferData(size_t index);
		void recordCommandBuffer(size_t index, VkCommandBuffer commandBuffer);

		ObjectInfo getObjectInfo() const;
		Matrix4f getMatrix() const;

		static size_t getRTDataSize();

		Vector3f scale;
		Rotation rotation;
		Vector3f position;

		Vector3f color;
		RTData rtData;

		float diffuseWeight;
		float reflectWeight;
		float transparentWeight;

		bool move;
		bool rotate;

		Vector3f moveStartPos;
		Vector3f moveStopPos;
		float moveSpeed;
		float movedDistance;

		Vector3f rotationAxis;
		Rotation totalRotation;
		float rotationSpeed;
		float rotatedAngle;
	
	private:
		const Mesh* mesh;
		const Device* device;
};
