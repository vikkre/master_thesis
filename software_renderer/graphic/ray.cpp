#include "ray.h"


Ray::Ray(const Vector3f& origin, const Vector3f& direction)
:origin(origin), direction(direction), directionInv(1.0f / direction) {}

Ray::~Ray() {}

void Ray::update() {
	directionInv = 1.0f / direction;
}
