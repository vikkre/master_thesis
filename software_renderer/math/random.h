#pragma once

#include <random>

#include "vector.h"


class RandomGenerator {
	public:
		RandomGenerator();
		~RandomGenerator();

		float rand();
		Vector3f randomNormal();
		Vector3f randomNormalDirection(const Vector3f& normal);

	private:
		std::mt19937 rng;
		std::uniform_real_distribution<float> distribution;
};
