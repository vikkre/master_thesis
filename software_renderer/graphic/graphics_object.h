#pragma once

#include "mesh.h"

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

		std::vector<size_t> indices;
		std::vector<Mesh::Vertex> vertices;

	private:
		const Mesh* mesh;
		Matrix4f objectMatrix;
};
