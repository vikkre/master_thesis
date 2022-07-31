#pragma once

#include "../math/vector.h"
#include "../math/matrix.h"


class Camera {
	public:
		Camera();

		Matrix4f getViewMatrix() const;
		Matrix4f getProjectionMatrix(float aspectRatio) const;

		Vector3f position;
		Vector3f lookAt;
		Vector3f up;

		float fov;
		float nearZ;
		float farZ;
		bool invertY;
};
