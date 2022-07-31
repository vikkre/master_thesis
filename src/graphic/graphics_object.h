#pragma once

#include <vulkan/vulkan.h>

#include "../math/vector.h"
#include "../math/matrix.h"
#include "../math/rotation.h"

#include <vector>


class Mesh;
class Device;
class Descriptor;

class GraphicsObject {
	public:
		struct ObjectData {
			Matrix4f objectMatrix;
			Vector3f color;
		};

		struct RTData {
			Matrix4f objectMatrix;
			Vector3f color;
			float reflect;
			uint64_t vertexAddress;
			uint64_t indexAddress;
		};

		struct ObjectInfo {
			VkAccelerationStructureInstanceKHR instance;
			void* dataPtr;
		};

		GraphicsObject(const Device* device, const Mesh* mesh, const Vector3f& position);
		~GraphicsObject();

		void init();
		void passBufferData(size_t index);
		void recordCommandBuffer(size_t index, const VkCommandBuffer* commandBuffer);

		ObjectInfo getObjectInfo() const;
		Matrix4f getMatrix() const;

		static std::vector<VkDescriptorSetLayoutBinding> getUniformBindings();
		static uint32_t getBindingSet();
		static size_t getRTDataSize();

		Vector3f scale;
		Rotation rotation;
		Vector3f position;

		ObjectData objectData;
		RTData rtData;
	
	private:
		void createDescriptorPool();
		void createDescriptors();

		const Mesh* mesh;
		const Device* device;

		VkDescriptorPool descriptorPool;
		std::vector<Descriptor> descriptors;
};
