#pragma once

#include <cmath>
#include <algorithm>
#include <thread>
#include <atomic>

#include "graphics_object.h"
#include "scene.h"

#include "../camera.h"
#include "../math/ray.h"
#include "../math/random.h"


class GraphicsEngine {
	public:
		enum class TraceResult {NO_HIT, HIT_DIFFUSE, HIT_REFLECT, HIT_TRANSPARENT};

		GraphicsEngine();
		~GraphicsEngine();

		void parseInput(const InputEntry& inputEntry);
		void init();
		void saveImage(const std::string path);

		void render();
		void render(const Matrix4f& viewInverse, const Matrix4f& projInverse, const Vector3f& origin);
		void renderPixel(unsigned int x, unsigned int y, const Matrix4f& viewInverse, const Matrix4f& projInverse, const Vector3f& origin);
		Vector3f traceRay(Ray ray);

		Vector2u imageSize;
		std::vector<char> image;
		Camera* camera;
		std::vector<GraphicsObject*> objects;
		Scene scene;
		RandomGenerator rng;

		unsigned int raysPerPixel;
		unsigned int visionJumpCount;
		unsigned int threadCount;
		std::atomic_uint32_t pixelCounter;
};
