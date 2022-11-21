#include "rotation.h"


Rotation::Rotation()
:s(1.0f), v({0.0f, 0.0f, 0.0f}) {}

Rotation::Rotation(Vector3f axis, float angle)
:s(cos(angle / 2.0f)), v(sin(angle / 2.0f) * axis.normalize()) {}


void Rotation::set(Vector3f axis, const float angle) {
	this->s = cos(angle / 2.0f);
	this->v = sin(angle / 2.0f) * axis.normalize();
}

float Rotation::getAngle() const {
	return 2.0f * acos(this->s);
}

Vector3f Rotation::getAxis() const {
	float angle_sin = sqrt(1 - this->s * this->s);
	return (1/angle_sin) * this->v;
}

Rotation Rotation::apply(const Rotation& other) const {
	Rotation ret;
	ret.s = (this->s * other.s) - this->v.dot(other.v);
	ret.v = (this->s * other.v) + (other.s * this->v) + cross(this->v, other.v);

	return ret;
}

Matrix4f Rotation::getMatrix() const {
	Matrix4f ret;

	if (v.magnitude() > 0) {
		float angle2 = this->s * this->s;
		float x2 = this->v[0] * this->v[0];
		float y2 = this->v[1] * this->v[1];
		float z2 = this->v[2] * this->v[2];
		
		ret[0][0] = angle2 + x2 - y2 - z2;
		ret[1][1] = angle2 - x2 + y2 - z2;
		ret[2][2] = angle2 - x2 - y2 + z2;
		ret[3][3] = angle2 + x2 + y2 + z2;

		ret[0][1] = 2.0f * (this->v[0] * this->v[1] + this->s * this->v[2]);
		ret[0][2] = 2.0f * (this->v[0] * this->v[2] - this->s * this->v[1]);
		ret[1][0] = 2.0f * (this->v[0] * this->v[1] - this->s * this->v[2]);
		ret[1][2] = 2.0f * (this->v[1] * this->v[2] + this->s * this->v[0]);
		ret[2][0] = 2.0f * (this->v[0] * this->v[2] + this->s * this->v[1]);
		ret[2][1] = 2.0f * (this->v[1] * this->v[2] - this->s * this->v[0]);
	}

	return ret;
}
