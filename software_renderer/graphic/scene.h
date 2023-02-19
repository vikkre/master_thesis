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
	
	private:
		std::vector<GraphicsObject*> objs;
		BVH bvh;
};
