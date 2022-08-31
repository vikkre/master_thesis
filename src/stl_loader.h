#pragma once

#include <iostream>

#include <cmath>
#include <cstdint>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <unordered_map>

#include "graphic/device.h"
#include "graphic/mesh.h"
#include "math/vector.h"


class STLLoader {
	public:
		STLLoader(const std::string& basepath);
		~STLLoader();

		void load(const std::string& path);
		Mesh* get_mesh(const Device* device) const;

		struct RawTriangle {
			float normal[3];
			float vertex[3][3];
			uint16_t attrib;
		} __attribute__((packed));

		struct Triangle {
			Vector3f normal;
			Vector3f vertex[3];
		};

		struct TrianglePoint {
			const Triangle* triangle;
			Vector3f pos;
			unsigned int vertex_index;
			unsigned int triangle_index;
		};

		struct IndexedTriangle {
			const Triangle* triangle;
			Vector3u indexes;
		};

	private:
		uint8_t header[80];
		uint32_t triangle_count;
		std::vector<Triangle> triangles;
		std::string basepath;
};
