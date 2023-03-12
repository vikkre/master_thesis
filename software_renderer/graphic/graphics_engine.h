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
		struct LightSourcePoint {
			Vector3f pos;
			Vector3f normal;
			Vector3f color;
			float lightStrength;
		};

		struct HitPoint {
			Vector3f pos;
			Vector3f normal;
			Vector3f cumulativeColor;
			bool diffuse;
			bool lightHit;
		};

		enum class TraceResult {NO_HIT, HIT_DIFFUSE, HIT_REFLECT, HIT_TRANSPARENT};

		GraphicsEngine();
		~GraphicsEngine();

		void parseInput(const InputEntry& inputEntry);
		void init();
		void saveImage(const std::string path);

		void render();
		void render(const Matrix4f& viewInverse, const Matrix4f& projInverse, const Vector3f& origin);
		void renderPixel(unsigned int x, unsigned int y, const Matrix4f& viewInverse, const Matrix4f& projInverse, const Vector3f& origin);
		size_t traceSinglePath(std::vector<HitPoint>& path, Ray ray, size_t startDepth, size_t maxDepth, bool isLightRay);

		LightSourcePoint getRandomLightSourcePoint();

		Vector2u imageSize;
		std::vector<char> image;
		Camera* camera;
		std::vector<GraphicsObject*> objects;
		std::vector<GraphicsObject*> lightSources;
		Scene scene;
		RandomGenerator rng;

		unsigned int raysPerPixel;
		unsigned int visionJumpCount;
		unsigned int lightJumpCount;
		unsigned int threadCount;
		std::atomic_uint32_t pixelCounter;
};
