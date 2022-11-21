#include "matrix.h"

Matrix4f getScaleMatrix(const Vector3f& scale) {
	Matrix4f ret;

	ret[0][0] = scale[0];
	ret[1][1] = scale[1];
	ret[2][2] = scale[2];

	return ret;
}

Matrix4f getTranslationMatrix(const Vector3f& position) {
	Matrix4f ret;

	ret[3][0] = position[0];
	ret[3][1] = position[1];
	ret[3][2] = position[2];

	return ret;
}

Matrix4f getLookAtMatrix(const Vector3f& position, const Vector3f& lookAt, const Vector3f& up) {
	Matrix4f ret;

	const Vector3f f = (lookAt - position).normalize();
	const Vector3f s = cross(f, up).normalize();
	const Vector3f u = cross(s, f);

	ret[0][0] =  s[0];
	ret[1][0] =  s[1];
	ret[2][0] =  s[2];
	ret[0][1] =  u[0];
	ret[1][1] =  u[1];
	ret[2][1] =  u[2];
	ret[0][2] = -f[0];
	ret[1][2] = -f[1];
	ret[2][2] = -f[2];
	ret[3][0] = -s.dot(position);
	ret[3][1] = -u.dot(position);
	ret[3][2] = f.dot(position);

	return ret;
}

Matrix4f getPerspectiveMatrix(const float aspect, const float fov, const float nearZ, const float farZ, bool invertY) {
	Matrix4f ret;

	const float s = tan(fov / 2.0f);

	ret[0][0] = 1.0f / (aspect * s);
	ret[1][1] = 1.0f / s;
	ret[2][2] = -(farZ + nearZ) / (farZ - nearZ);
	ret[2][3] = -1.0f;
	ret[3][2] = -(2.0f * farZ * nearZ) / (farZ - nearZ);
	ret[3][3] = 0.0f;

	if (invertY) ret[1][1] *= -1;

	return ret;
}
