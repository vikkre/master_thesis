#pragma once 

#include <vulkan/vulkan.h>

#include <vector>

#include "helper/data_buffer.h"
#include "helper/bottom_acceleration_structure.h"

#include "../init_exception.h"
#include "../math/vector.h"


class Device;

class Mesh {
	public:
		struct Vertex {
			Vector3f point;
			Vector3f normal;
		};
	
		Mesh(const Device* device);
		~Mesh();

		void init();
		
		void addPoint(const Vector3f& point, const Vector3f& normal);
		void addIndex(const Vector3u& index);

		const DataBuffer& getVertexBuffer() const;
		const DataBuffer& getIndexBuffer() const;
		const BottomAccelerationStructure& getBlas() const;
		float getWeight() const;

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

	private:
		void createVertexBuffer();
		void createIndexBuffer();
		void createBlas();
		void calcWeight();

		const Device* device;

		DataBuffer vertexBuffer;
		DataBuffer indexBuffer;

		std::vector<VkBuffer> vertexBuffers;
		std::vector<VkDeviceSize> vertexBufferOffsets;
		BottomAccelerationStructure blas;
		float weight;
};
