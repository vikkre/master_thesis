#include "stl_loader.h"

#define ANGLE_THREASHOLD 1.0/3.0 * M_PI

#define STL_PATH std::string("../res/stl/")


STLLoader::STLLoader(const std::string& basepath)
:header(), triangle_count(), triangles(), basepath(basepath) {}

STLLoader::~STLLoader() {}

bool fits(Vector3f n1, Vector3f n2) {
	float angle = n1.angle(n2);
	return angle < ANGLE_THREASHOLD;
}

void STLLoader::load(const std::string& path) {
	triangles.clear();

	std::ifstream file(STL_PATH + path, std::ifstream::in | std::ifstream::binary);
	if (!file.is_open()) {
		throw InitException("STLLoader", std::string("Could not load STL file \"") + path + "\"!");
	}

	file.read((char*) header, 80);
	file.read((char*) &triangle_count, 4);

	for (size_t i = 0; i < triangle_count; ++i) {
		RawTriangle raw;
		file.read((char*) &raw, sizeof(RawTriangle));

		Triangle triangle;
		triangle.normal = Vector3f({raw.normal[0], raw.normal[2], raw.normal[1]});
		for (size_t i = 0; i < 3; ++i) {
			triangle.vertex[i] = Vector3f({raw.vertex[i][0], raw.vertex[i][2], raw.vertex[i][1]});
		}
		triangles.push_back(triangle);
	}

	file.close();
}

Mesh* STLLoader::get_mesh() const {
	Mesh* mesh = new Mesh();

	std::unordered_map<Vector3f, std::vector<TrianglePoint>> pre_groups;
	std::vector<std::pair<Vector3f, std::vector<TrianglePoint>>> post_groups;
	std::vector<IndexedTriangle> indexed_triangles(triangles.size());

	for (unsigned int triangle_index = 0; triangle_index < triangles.size(); ++triangle_index) {
		const Triangle* triangle = &triangles.at(triangle_index);
		
		for (unsigned int i = 0; i < 3; ++i) {
			Vector3f pos = triangle->vertex[i];

			if (pre_groups.count(pos) == 0) {
				pre_groups.insert({pos, {}});
			}

			TrianglePoint tp;
			tp.triangle = triangle;
			tp.pos = pos;
			tp.vertex_index = i;
			tp.triangle_index = triangle_index;
			pre_groups[pos].push_back(tp);
		}

		indexed_triangles.at(triangle_index).triangle = triangle;
	}

	for (const std::pair<Vector3f, std::vector<TrianglePoint>> kv: pre_groups) {
		const Vector3f pos = kv.first;
		std::vector<TrianglePoint> pre_triangles = kv.second;

		std::vector<std::vector<TrianglePoint>> post_triangles(0);
		post_triangles.push_back({pre_triangles.back()});
		pre_triangles.pop_back();

		for (TrianglePoint& pre_tp: pre_triangles) {
			bool is_added = false;
			for (std::vector<TrianglePoint>& tp_group: post_triangles) {
				if (fits(tp_group.at(0).triangle->normal, pre_tp.triangle->normal)) {
					tp_group.push_back(pre_tp);
					is_added = true;
					break;
				}
			}
			if (!is_added) {
				post_triangles.push_back({pre_tp});
			}
		}

		for (std::vector<TrianglePoint>& tp_group: post_triangles) {
			post_groups.push_back({pos, tp_group});
		}
	}

	unsigned int index = 0;
	for (const std::pair<Vector3f, std::vector<TrianglePoint>> kv: post_groups) {
		const Vector3f pos = kv.first;
		std::vector<TrianglePoint> group_triangles = kv.second;

		Vector3f normal;
		for (TrianglePoint tp: group_triangles) {
			normal += tp.triangle->normal;
		}
		normal /= group_triangles.size();

		mesh->addVertex(pos, normal);
		for (TrianglePoint tp: group_triangles) {
			IndexedTriangle& it = indexed_triangles.at(tp.triangle_index);
			it.indexes[tp.vertex_index] = index;
		}
		++index;
	}

	for (const IndexedTriangle& it: indexed_triangles) {
		mesh->addIndex(it.indexes);
	}

	return mesh;
}
