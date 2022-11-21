#pragma once

#include "graphics_object.h"

#include "../math/vector.h"
#include "../math/matrix.h"

class Triangle {
	public:
		Triangle(const GraphicsObject* obj, size_t v0, size_t v1, size_t v2);
		~Triangle();

		bool rayIntersects(const Vector3f& rayOrigin, const Vector3f& rayDirection, Vector3f& outIntersectionPoint) const;
		Vector3f getBarycentricCoords(const Vector3f& outIntersectionPoint) const;

		const GraphicsObject* obj;

		const size_t v0;
		const size_t v1;
		const size_t v2;

	private:
		Vector3f edge0;
		Vector3f edge1;

		float d00;
		float d01;
		float d11;
		float denom;
};
