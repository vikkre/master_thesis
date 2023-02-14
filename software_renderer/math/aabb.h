#pragma once

#include "vector.h"
#include "../graphic/ray.h"


class AABB {
	public:
		AABB();
		AABB(const Vector3f& aabbMin, const Vector3f& aabbMax);
		AABB(const Vector3f& point);
		AABB(const AABB& a, const AABB& b);
		~AABB();

		void addPoint(const Vector3f& point);

		bool doesRayIntersect(const Ray& ray) const;

	private:
		bool empty;
		Vector3f aabbMin;
		Vector3f aabbMax;
};
