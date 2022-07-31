#include "camera.h"


Camera::Camera()
:position({0.0f, 0.0f, 0.0f}), lookAt({0.0f, 0.0f, -1.0f}), up({0.0f, 1.0f, 0.0f}),
fov(M_PI_4), nearZ(0.01f), farZ(1000.0f), invertY(true) {}

Matrix4f Camera::getViewMatrix() const {
	return getLookAtMatrix(position, lookAt, up);
}

Matrix4f Camera::getProjectionMatrix(float aspectRatio) const {
	return getPerspectiveMatrix(aspectRatio, fov, nearZ, farZ, invertY);
}
