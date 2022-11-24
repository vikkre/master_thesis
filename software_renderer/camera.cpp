#include "camera.h"

#define MOVE_SPEED  7.0f
#define ANGLE_SPEED 0.01f

#define PHI_MIN       0.01f
#define PHI_MAX       M_PI - 0.01f
#define THETA_MIN    -M_PI
#define THETA_MAX     M_PI
#define THETA_CHANGE  M_PI * 2.0f


Camera::Camera()
:theta(M_PI), phi(M_PI_2), position({4.0f, 0.0f, 0.0f}),
fov(M_PI_4), nearZ(0.01f), farZ(1000.0f), invertY(true) {}

Camera::~Camera() {}

void Camera::parseInput(const InputEntry& inputEntry) {
	position = inputEntry.getVector<3, float>("position");
	theta = inputEntry.get<float>("angle", 0);
	phi = inputEntry.get<float>("angle", 1);

	theta *= M_PI;
	phi = phi * -M_PI_2 + M_PI_2;

	if (phi < PHI_MIN) phi = PHI_MIN;
	if (phi > PHI_MAX) phi = PHI_MAX;
	if (theta < THETA_MIN) theta += THETA_CHANGE;
	if (theta > THETA_MAX) theta -= THETA_CHANGE;
}

Matrix4f Camera::getViewMatrix() const {
	float sinPhi = sin(phi);
	float cosPhi = cos(phi);
	float sinTheta = sin(theta);
	float cosTheta = cos(theta);

	Vector3f lookDirection = Vector3f({
		sinPhi * cosTheta,
		cosPhi,
		sinPhi * sinTheta
	});
	Vector3f up({0.0f, 1.0f, 0.0f});

	return getLookAtMatrix(position, lookDirection + position, up);
}

Matrix4f Camera::getProjectionMatrix(float aspectRatio) const {
	return getPerspectiveMatrix(aspectRatio, fov, nearZ, farZ, invertY);
}
