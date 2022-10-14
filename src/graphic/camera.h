#pragma once

#include "../math/vector.h"
#include "../math/matrix.h"

#include "../input/input.h"


class Camera {
	public:
		Camera();

		Matrix4f getViewMatrix() const;
		Matrix4f getProjectionMatrix(float aspectRatio) const;

		Input* input;

		float fov;
		float nearZ;
		float farZ;
		bool invertY;
};
