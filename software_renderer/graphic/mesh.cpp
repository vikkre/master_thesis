#include "mesh.h"


Mesh::Mesh()
:vertices(), indices() {}

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
