#include "scene.h"


Scene::Scene()
:objs() {}

Scene::~Scene() {}

void Scene::addObject(GraphicsObject* obj) {
	objs.push_back(obj);
}

void Scene::init() {
	std::vector<BVH::Data> inputs;
	inputs.reserve(objs.size());
	for (size_t i = 0; i < objs.size(); ++i) {
		inputs.push_back({objs[i]->aabb, i});
	}
	bvh.init(inputs);
}

bool Scene::traceRay(const Ray& ray, Mesh::Vertex& hitVertex, const GraphicsObject*& currentObj) const {
	std::vector<size_t> elems = bvh.getHits(ray);
	if (elems.empty()) return false;

	std::vector<GraphicsObject*> potentialHits(elems.size());
	for (size_t i = 0; i < elems.size(); ++i) {
		potentialHits[i] = objs[elems[i]];
	}

	float minDistance = INFINITY;
	const Triangle* currentTriangle = nullptr;
	currentObj = nullptr;
	Vector3f hitPos;
	for (const GraphicsObject* obj: potentialHits) {
		if (obj->traceRay(ray, hitPos, currentTriangle, minDistance)) {
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
	hitVertex.normal.normalize();

	return true;
}
