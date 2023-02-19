#include "mesh.h"


Mesh::Mesh()
:vertices(), indices(), bvh() {}

Mesh::~Mesh() {}

void Mesh::addVertex(const Vector3f& pos, const Vector3f& normal) {
	Mesh::Vertex vertex{};

	vertex.pos = pos;
	vertex.normal = normal;

	vertices.push_back(vertex);
}

void Mesh::addIndex(const Vector3u& index) {
	indices.push_back(index[0]);
	indices.push_back(index[1]);
	indices.push_back(index[2]);
}

void Mesh::init() {
	std::vector<BVH::Data> inputs;
	inputs.reserve((indices.size() / 3));
	
	for (size_t i = 0; i < indices.size(); i += 3) {
		AABB aabb;
		aabb.addPoint(vertices[indices[i+0]].pos);
		aabb.addPoint(vertices[indices[i+1]].pos);
		aabb.addPoint(vertices[indices[i+2]].pos);

		inputs.push_back({aabb, i / 3});
	}
	bvh.init(inputs);
}
