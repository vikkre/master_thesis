#pragma once


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
		Vector3f renderPixel(Vector3f origin, Vector3f direction);

		Vector2u imageSize;
		std::vector<char> image;
		Camera* camera;
		std::vector<GraphicsObject*> objects;
		Scene scene;
		RandomGenerator rng;

		unsigned int raysPerPixel;
		unsigned int visionJumpCount;
};
