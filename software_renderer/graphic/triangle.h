#pragma once

#include "../math/vector.h"
#include "../math/matrix.h"


class Triangle {
	public:
		Triangle(const Vector3u& indices, const Vector3f& v0, const Vector3f& v1, const Vector3f& v2);
		~Triangle();

		bool rayIntersects(const Vector3f& rayOrigin, const Vector3f& rayDirection, Vector3f& outIntersectionPoint) const;
		Vector3f getBarycentricCoords(const Vector3f& outIntersectionPoint) const;

		const Vector3u indices;
		const Vector3f v0;
		const Vector3f v1;
		const Vector3f v2;

	private:
		Vector3f edge0;
		Vector3f edge1;

		float d00;
		float d01;
		float d11;
		float denom;
};
