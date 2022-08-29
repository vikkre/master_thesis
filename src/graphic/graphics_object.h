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
		};

		struct ObjectInfo {
			VkAccelerationStructureInstanceKHR instance;
			void* dataPtr;
		};

		GraphicsObject(const Device* device, const Mesh* mesh, const Vector3f& position);
		~GraphicsObject();

		void init();
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
	
	private:
		const Mesh* mesh;
		const Device* device;
};
