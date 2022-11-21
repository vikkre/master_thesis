#include "random.h"


RandomGenerator::RandomGenerator()
:rng(std::random_device()()), distribution(0.0f, 1.0f) {}

RandomGenerator::~RandomGenerator() {}

float RandomGenerator::rand() {
	return distribution(rng);
}

Vector3f RandomGenerator::randomNormal() {
	float theta = 2.0f * M_PI * rand();
	float u = 2.0f * rand() - 1.0f;
	float suu = sqrt(1.0f - u*u);

	float sinTheta = sin(theta);
	float cosTheta = cos(theta);

	return Vector3f({
		suu * cosTheta,
		suu * sinTheta,
		u
	});
}

Vector3f RandomGenerator::randomNormalDirection(const Vector3f& normal) {
	Vector3f randVec = randomNormal();
	if (randVec.dot(normal) < 0.0f) randVec *= -1.0f;
	return randVec;
}
