#include "aabb.h"


AABB::AABB()
:empty(true), aabbMin(), aabbMax() {}

AABB::AABB(const Vector3f& aabbMin, const Vector3f& aabbMax)
:empty(false), aabbMin(aabbMin), aabbMax(aabbMax) {}

AABB::AABB(const Vector3f& point)
:empty(false), aabbMin(point), aabbMax(point + 0.0001f) {}

AABB::AABB(const AABB& a, const AABB& b)
:empty(false), aabbMin(), aabbMax() {
	for (size_t i = 0; i < 3; ++i) {
		aabbMin[i] = std::min(a.aabbMin[i], b.aabbMin[i]);
		aabbMax[i] = std::max(a.aabbMax[i], b.aabbMax[i]);
	}
}

AABB::~AABB() {}

void AABB::addPoint(const Vector3f& point) {
	if (empty) {
		aabbMin = point;
		aabbMax = point + 0.0001f;
		empty = false;
	} else {
		for (size_t i = 0; i < 3; ++i) {
			aabbMin[i] = std::min(aabbMin[i], point[i]);
			aabbMax[i] = std::max(aabbMax[i], point[i]);
		}
	}
}

bool AABB::doesRayIntersect(const Ray& ray) const {
	if (empty) return false;

	float tmin = 0.0, tmax = INFINITY;

	for (size_t i = 0; i < 3; ++i) {
		float t1 = (aabbMin[i] - ray.origin[i]) * ray.directionInv[i];
		float t2 = (aabbMax[i] - ray.origin[i]) * ray.directionInv[i];

		tmin = std::max(tmin, std::min(t1, t2));
		tmax = std::min(tmax, std::max(t1, t2));
	}

	return tmin < tmax;
}
