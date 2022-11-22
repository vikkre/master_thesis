#include "scene.h"


Scene::Scene()
:objs() {}

Scene::~Scene() {}

void Scene::addObject(GraphicsObject* obj) {
	objs.push_back(obj);
}

bool Scene::traceRay(const Vector3f& rayOrigin, const Vector3f& rayDirection, Mesh::Vertex& hitVertex, const GraphicsObject*& currentObj) const {
	float minDistance = INFINITY;
	const Triangle* currentTriangle = nullptr;
	currentObj = nullptr;
	Vector3f hitPos;
	for (const GraphicsObject* obj: objs) {
		if (obj->traceRay(rayOrigin, rayDirection, hitPos, currentTriangle, minDistance)) {
			currentObj = obj;
		}
	}

	if (currentObj == nullptr) return false;

	Vector3f barycentricCoords = currentTriangle->getBarycentricCoords(hitPos);
	hitVertex.pos = hitPos;

	const Mesh::Vertex& vert0 = currentObj->vertices[currentTriangle->indices[0]];
	const Mesh::Vertex& vert1 = currentObj->vertices[currentTriangle->indices[1]];
	const Mesh::Vertex& vert2 = currentObj->vertices[currentTriangle->indices[2]];
	hitVertex.normal = Vector3f({0.0f, 0.0f, 0.0f});
	hitVertex.normal += barycentricCoords[0] * vert0.normal;
	hitVertex.normal += barycentricCoords[1] * vert1.normal;
	hitVertex.normal += barycentricCoords[2] * vert2.normal;

	return true;
}
