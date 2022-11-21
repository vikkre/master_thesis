#include "scene.h"


Scene::Scene()
:triangles() {}

Scene::~Scene() {}

void Scene::addObject(GraphicsObject* obj) {
	for (size_t i = 0; i < obj->indices.size(); i += 3) {
		triangles.emplace_back(obj,
			obj->indices[i+0],
			obj->indices[i+1],
			obj->indices[i+2]
		);
	}
}

bool Scene::traceRay(const Vector3f& rayOrigin, const Vector3f& rayDirection, Mesh::Vertex& hitVertex, const GraphicsObject*& obj) const {
	float minDistance = INFINITY;
	const Triangle* currentTriangle = nullptr;
	Vector3f hitPos;
	for (const Triangle& triangle: triangles) {
		Vector3f currentHitPos;
		if (triangle.rayIntersects(rayOrigin, rayDirection, currentHitPos)) {
			float currentDistance = rayOrigin.distanceSquared(currentHitPos);
			if (currentDistance < minDistance) {
				hitPos = currentHitPos;
				minDistance = currentDistance;
				currentTriangle = &triangle;
			}
		}
	}

	if (currentTriangle == nullptr) return false;

	Vector3f barycentricCoords = currentTriangle->getBarycentricCoords(hitPos);
	hitVertex.pos = hitPos;
	obj = currentTriangle->obj;

	Mesh::Vertex vert0 = obj->vertices[currentTriangle->v0];
	Mesh::Vertex vert1 = obj->vertices[currentTriangle->v1];
	Mesh::Vertex vert2 = obj->vertices[currentTriangle->v2];
	hitVertex.normal = Vector3f({0.0f, 0.0f, 0.0f});
	hitVertex.normal += barycentricCoords[0] * vert0.normal;
	hitVertex.normal += barycentricCoords[1] * vert1.normal;
	hitVertex.normal += barycentricCoords[2] * vert2.normal;

	return true;
}
