#pragma once

#include <iostream>

#include "vector.h"
#include "matrix.h"


class Rotation {
	public:
		Rotation();
		Rotation(Vector3f axis, const float angle);

		void set(Vector3f axis, const float angle);

		float getAngle() const;
		Vector3f getAxis() const;

		Rotation apply(const Rotation& other) const;
		Matrix4f getMatrix() const;

	private:
		float s;
		Vector3f v;
};


std::ostream& operator<<(std::ostream& out, const Rotation& r);
