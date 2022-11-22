#pragma once

#include "mesh.h"
#include "triangle.h"

#include "../math/vector.h"
#include "../math/matrix.h"
#include "../math/rotation.h"

#include <vector>


class Mesh;
class Device;

class GraphicsObject {
	public:
		GraphicsObject(const Mesh* mesh, const Vector3f& position);
		~GraphicsObject();

		void init();
		Matrix4f getMatrix() const;
		bool traceRay(const Vector3f& rayOrigin, const Vector3f& rayDirection, Vector3f& hitPos, const Triangle*& currentTriangle, float& minDistance) const;

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

	private:
		const Mesh* mesh;
		Matrix4f objectMatrix;
		std::vector<Triangle> triangles;
		Vector3f aabbMin;
		Vector3f aabbMax;
};
