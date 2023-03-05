#pragma once

#include "graphics_object.h"
#include "triangle.h"

#include "../math/ray.h"
#include "../math/bounding_volume_hierachy.h"


class Scene {
	public:
		Scene();
		~Scene();

		void addObject(GraphicsObject* obj);
		void init();
		bool traceRay(const Ray& ray, Mesh::Vertex& hitVertex, const GraphicsObject*& currentObj) const;
		bool isOccluded(const Vector3f& startPos, const Vector3f& endPos) const;
	
	private:
		std::vector<GraphicsObject*> objs;
		BVH bvh;
};
