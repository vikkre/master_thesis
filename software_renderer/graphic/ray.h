#pragma once

#include "../math/vector.h"


class Ray {
	public:
		Ray(const Vector3f& origin, const Vector3f& direction);
		~Ray();

		void update();

		Vector3f origin;
		Vector3f direction;
		Vector3f directionInv;
};
