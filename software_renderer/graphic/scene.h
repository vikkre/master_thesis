#pragma once

#include "graphics_object.h"
#include "triangle.h"


class Scene {
	public:
		Scene();
		~Scene();

		void addObject(GraphicsObject* obj);
		bool traceRay(const Vector3f& rayOrigin, const Vector3f& rayDirection, Mesh::Vertex& hitVertex, const GraphicsObject*& currentObj) const;
	
	private:
		std::vector<GraphicsObject*> objs;
};