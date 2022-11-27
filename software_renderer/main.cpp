#include <iostream>
#include <filesystem>

#include "graphic/graphics_engine.h"
#include "graphic/mesh.h"
#include "graphic/graphics_object.h"

#include "init_exception.h"
#include "mesh_manager.h"
#include "input_parser.h"
#include "camera.h"

#include "math/vector.h"
#include "math/matrix.h"
#include "math/rotation.h"


int main(int argc, char* argv[]) {
	// Vector3f direction({1.0f, -1.0f, 0.0f});
	// Vector3f normal({0.0f, 1.0f, 0.0f});
	// Vector3f refl = reflect(direction, normal);

	// std::cout << direction << std::endl;
	// std::cout << normal << std::endl;
	// std::cout << refl << std::endl;

	// return 0;


	if (argc != 7) {
		std::cout << "Error: wrong paramter count!" << std::endl;
		std::cout << "Usage: SoftwareRenderer renderer scene image_width image_height camera resultimage" << std::endl;
		return -1;
	}

	const std::string execpath = argv[0];
	const std::string basepath = execpath.substr(0, execpath.size() - sizeof("SoftwareRenderer") + 1);

	std::string rendererPath = argv[1];
	std::string scenePath = argv[2];
	Vector2u imageSize = Vector2u({(unsigned int) std::atoi(argv[3]), (unsigned int) std::atoi(argv[4])});
	std::string cameraFilePath = argv[5];
	std::string resultImagePath = argv[6];

	size_t start = rendererPath.find_last_of('/') + 1;
	size_t finish = rendererPath.find_last_of('.') - start;
	std::string rendererName = rendererPath.substr(start, finish);


	GraphicsEngine* engine = new GraphicsEngine();
	engine->imageSize = imageSize;

	MeshManager* meshManager = new MeshManager(basepath);
	meshManager->createObjectsFromFile(scenePath);

	InputParser rendererParser(rendererPath);
	rendererParser.parse();

	engine->parseInput(rendererParser.getInputEntry(0));
	engine->objects = meshManager->getCreatedObjects();

	engine->init();

	Camera* camera = new Camera();
	InputParser cameraParser(cameraFilePath);
	cameraParser.parse();
	camera->parseInput(cameraParser.getInputEntry(0));
	engine->camera = camera;

	engine->render();
	engine->saveImage(resultImagePath);

	delete camera;
	delete meshManager;
	delete engine;

	return 0;
}
