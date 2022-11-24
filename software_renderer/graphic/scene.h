#pragma once

#include "graphics_object.h"
#include "triangle.h"
#include "ray.h"


class Scene {
	public:
		Scene();
		~Scene();

		void addObject(GraphicsObject* obj);
		bool traceRay(const Ray& ray, Mesh::Vertex& hitVertex, const GraphicsObject*& currentObj) const;
	
	private:
		std::vector<GraphicsObject*> objs;
};
