#pragma once

#include <thread>

#include "graphics_object.h"
#include "scene.h"

#include "../camera.h"
#include "../math/random.h"


class GraphicsEngine {
	public:
		GraphicsEngine();
		~GraphicsEngine();

		void parseInput(const InputEntry& inputEntry);
		void init();
		void saveImage(const std::string path);

		void render();
		void render(unsigned int t, unsigned int startY, unsigned int endY, const Matrix4f& viewInverse, const Matrix4f& projInverse, const Vector3f& origin);
		void renderPixel(unsigned int x, unsigned int y, const Matrix4f& viewInverse, const Matrix4f& projInverse, const Vector3f& origin);
		Vector3f renderRay(Vector3f origin, Vector3f direction);

		Vector2u imageSize;
		std::vector<char> image;
		Camera* camera;
		std::vector<GraphicsObject*> objects;
		Scene scene;
		RandomGenerator rng;

		unsigned int raysPerPixel;
		unsigned int visionJumpCount;
		unsigned int threadCount;
};
