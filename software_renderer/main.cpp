#include <iostream>
#include <filesystem>

#include "graphic/graphics_engine.h"
#include "graphic/mesh.h"
#include "graphic/graphics_object.h"

#include "graphic/renderer.h"
#include "graphic/path_tracer.h"
#include "graphic/bidirectional_path_tracer.h"

#include "init_exception.h"
#include "mesh_manager.h"
#include "input_parser.h"
#include "camera.h"

#include "math/vector.h"
#include "math/matrix.h"
#include "math/rotation.h"


Renderer* getRenderer(const std::string& name) {
	if (name == "PathTracer")              return new PathTracer();
	if (name == "BidirectionalPathTracer") return new BidirectionalPathTracer();
	else throw InitException("getRenderer not found", name);
}

int main(int argc, char* argv[]) {
	if (argc != 8) {
		std::cout << "Error: wrong paramter count!" << std::endl;
		std::cout << "Usage: SoftwareRenderer renderer scene image_width image_height thread_count camera resultimage" << std::endl;
		return -1;
	}

	std::cout << argv[0];
	for (unsigned int i = 1; i < 7; ++i) {
		std::cout << " " << argv[i];
	}
	std::cout << std::endl;

	const std::string execpath = argv[0];
	const std::string basepath = execpath.substr(0, execpath.size() - sizeof("SoftwareRenderer") + 1);

	std::string rendererPath = argv[1];
	std::string scenePath = argv[2];
	Vector2u imageSize = Vector2u({(unsigned int) std::atoi(argv[3]), (unsigned int) std::atoi(argv[4])});
	unsigned int threadCount = (unsigned int) std::atoi(argv[5]);
	std::string cameraFilePath = argv[6];
	std::string resultImagePath = argv[7];

	size_t start = rendererPath.find_last_of('/') + 1;
	size_t finish = rendererPath.find_last_of('.') - start;
	std::string rendererName = rendererPath.substr(start, finish);


	GraphicsEngine* engine = new GraphicsEngine();
	engine->imageSize = imageSize;

	MeshManager* meshManager = new MeshManager(basepath);
	meshManager->createObjectsFromFile(scenePath);

	InputParser rendererParser(rendererPath);
	rendererParser.parse();

	Renderer* renderer = getRenderer(rendererParser.getInputEntry(0).name);
	renderer->parseInput(rendererParser.getInputEntry(0));

	engine->objects = meshManager->getCreatedObjects();
	engine->lightSources = meshManager->getCreatedLightSources();

	engine->init(renderer, threadCount);

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
