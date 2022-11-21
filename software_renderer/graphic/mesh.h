#pragma once 



#include <vector>

#include "../init_exception.h"
#include "../math/vector.h"


class Device;

class Mesh {
	public:
		struct Vertex {
			Vector3f pos;
			Vector3f normal;
		};
	
		Mesh();
		~Mesh();

		void addVertex(const Vector3f& pos, const Vector3f& normal);
		void addIndex(const Vector3u& index);

		std::vector<Vertex> vertices;
		std::vector<size_t> indices;
};
