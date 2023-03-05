#pragma once

#include "mesh.h"
#include "triangle.h"

#include "../math/ray.h"
#include "../math/vector.h"
#include "../math/matrix.h"
#include "../math/rotation.h"
#include "../math/aabb.h"
#include "../math/bounding_volume_hierachy.h"

#include <vector>


class Mesh;
class Device;

class GraphicsObject {
	public:
		GraphicsObject(const Mesh* mesh, const Vector3f& position);
		~GraphicsObject();

		void init();
		Matrix4f getMatrix() const;
		bool traceRay(const Ray& ray, Vector3f& hitPos, const Triangle*& currentTriangle, float& minDistance) const;

		Vector3f scale;
		Rotation rotation;
		Vector3f position;

		Vector3f color;
		bool lightSource;
		float lightStrength;

		float diffuseWeight;
		float reflectWeight;
		float transparentWeight;

		float diffuseThreshold;
		float reflectThreshold;
		float transparentThreshold;

		float refractionIndex;

		std::vector<Mesh::Vertex> vertices;
		AABB aabb;

		const Mesh* mesh;
		Matrix4f objectMatrix;
		std::vector<Triangle> triangles;
		BVH bvh;
};
