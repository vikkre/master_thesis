#include "triangle.h"


Triangle::Triangle(const Vector3u& indices, const Vector3f& v0, const Vector3f& v1, const Vector3f& v2)
:indices(indices), v0(v0), v1(v1), v2(v2) {
	edge0 = v1 - v0;
	edge1 = v2 - v0;

	d00 = edge0.dot(edge0);
	d01 = edge0.dot(edge1);
	d11 = edge1.dot(edge1);
	denom = d00 * d11 - d01 * d01;
}

Triangle::~Triangle() {}

bool Triangle::rayIntersects(const Vector3f& rayOrigin, const Vector3f& rayDirection, Vector3f& outIntersectionPoint) const {
	constexpr float EPSILON = 0.0000001f;
	
	Vector3f h = cross(rayDirection, edge1);
	float a = edge0.dot(h);
	if (a > -EPSILON && a < EPSILON) return false;

	float f = 1.0f / a;
	Vector3f s = rayOrigin - v0;
	float u = f * s.dot(h);
	if (u < 0.0f || u > 1.0f) return false;

	Vector3f q = cross(s, edge0);
	float v = f * rayDirection.dot(q);
	if (v < 0.0f || u + v > 1.0f) return false;

	float t = f * edge1.dot(q);
	if (t > EPSILON) {
		outIntersectionPoint = rayOrigin + (t * rayDirection);
		return true;

	} else {
		return false;
	}
}

Vector3f Triangle::getBarycentricCoords(const Vector3f& outIntersectionPoint) const {
	Vector3f edge2 = outIntersectionPoint - v0;
	float d20 = edge2.dot(edge0);
	float d21 = edge2.dot(edge1);

	float v = (d11 * d20 - d01 * d21) / denom;
	float w = (d00 * d21 - d01 * d20) / denom;
	return Vector3f({1.0f - v - w, v, w});
}
