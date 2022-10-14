#include "camera.h"


Camera::Camera()
:input(nullptr), fov(M_PI_4), nearZ(0.01f), farZ(1000.0f), invertY(true) {}

Matrix4f Camera::getViewMatrix() const {
	Vector3f position({0.0f, 0.0f, 0.0f});
	Vector3f lookAt({0.0f, 0.0f, 0.0f});
	Vector3f up({0.0f, 1.0f, 0.0f});

	if (input != nullptr) {
		position = input->getPosition();
		lookAt = input->getLookAt();
		up = input->getUp();
	}

	return getLookAtMatrix(position, lookAt, up);
}

Matrix4f Camera::getProjectionMatrix(float aspectRatio) const {
	return getPerspectiveMatrix(aspectRatio, fov, nearZ, farZ, invertY);
}
