#include <SDL2/SDL.h>
#include <iostream>
#include <map>
#include <algorithm>
#include <chrono>
#include <functional>
#include <stdio.h>
#include <unistd.h>

#include "graphic/device.h"
#include "graphic/graphics_engine.h"
#include "graphic/mesh.h"
#include "graphic/graphics_object.h"
#include "graphic/helper/top_acceleration_structure_buffer.h"
#include "graphic/renderer/path_tracer.h"
#include "graphic/renderer/majercik2019_renderer.h"
#include "graphic/renderer/meta_renderer.h"
#include "graphic/renderer/phong_renderer.h"
#include "graphic/denoiser/gauss_denoiser.h"
#include "graphic/denoiser/median_denoiser.h"

#include "init_exception.h"
#include "mesh_manager.h"
#include "input_parser.h"

#include "input/input.h"
#include "input/fake_input.h"
#include "input/spherical_input.h"
#include "input/free_input.h"

#include "math/vector.h"
#include "math/matrix.h"
#include "math/rotation.h"


constexpr Uint32 FPS = 30;
constexpr Uint32 MS_PER_FRAME = 1000/FPS;


int64_t measureExecTimeMicroseconds(std::function<void()> exec) {
	auto start = std::chrono::high_resolution_clock::now();
	exec();
	auto stop = std::chrono::high_resolution_clock::now();

	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
	return duration.count();
}

float microsecondsToSeconds(int64_t microseconds) {
	return float(microseconds) / (1000.0f * 1000.0f);
}

Renderer* getRenderer(const std::string& name, Device* device) {
	if (name == "PathTracer") return new PathTracer(device);
	if (name == "Majercik2019") return new Majercik2019(device);
	if (name == "MetaRenderer") return new MetaRenderer(device);
	if (name == "PhongShader") return new PhongRenderer(device);
	else return nullptr;
}

Denoiser* getDenoiser(const std::string& name, Device* device) {
	if      (name == "GaussDenoiser")  return new GaussDenoiser(device);
	else if (name == "MedianDenoiser") return new MedianDenoiser(device);
	else return nullptr;
}


int main(int argc, char* argv[]) {
	if (argc != 5 && argc != 7) {
		std::cout << "Error: wrong paramter count!" << std::endl;
		std::cout << "Usage: RayTrace renderer scene image_width image_height [resultimage renderrounds]" << std::endl;
		return -1;
	}

	const std::string execpath = argv[0];
	const std::string basepath = execpath.substr(0, execpath.size() - sizeof("RayTrace") + 1);

	std::string rendererPath = argv[1];
	std::string scenePath = argv[2];
	Vector2i windowSize = Vector2i({std::atoi(argv[3]), std::atoi(argv[4])});
	std::string resultImagePath;
	unsigned int maxRenderCount;

	bool renderLimit = argc == 7;
	if (renderLimit) {
		resultImagePath = argv[5];
		maxRenderCount = std::atoi(argv[6]);
	}


	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		InitException("SDL_Init", SDL_GetError());
	}
	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);

	GraphicsEngine* engine = new GraphicsEngine(basepath);
	engine->windowSize = windowSize;
	engine->init();

	engine->device.renderInfo.lightPosition = Vector3f({0.0f, 4.5f, 0.0f});

	MeshManager* meshManager = new MeshManager(&engine->device, basepath);

	meshManager->createObjectsFromFile(scenePath);

	InputParser rendererParser(rendererPath);
	rendererParser.parse();

	engine->renderer = getRenderer(rendererParser.getInputEntry(0).name, &engine->device);
	engine->renderer->parseInput(rendererParser.getInputEntry(0));
	std::vector<GraphicsObject*> objs = meshManager->getCreatedObjects();
	engine->renderer->passObjects(objs);

	for (unsigned int i = 1; i < rendererParser.size(); ++i) {
		const InputEntry& inputEntry = rendererParser.getInputEntry(i);
		Denoiser* denoiser = getDenoiser(inputEntry.name, &engine->device);
		denoiser->parseInput(inputEntry);
		engine->denoisers.push_back(denoiser);
	}

	engine->initTlas();

	const bool useSphericalCamera = false;
	Input* input = nullptr;
	if (renderLimit) {
		FakeInput* nInput = new FakeInput(Vector3f({22.0f, 0.0f, 0.0f}));
		input = nInput;
	} else if (useSphericalCamera) {
		SphericalInput* rInput = new SphericalInput();
		rInput->r = 15.0f;
		input = rInput;
	} else {
		FreeInput* fInput = new FreeInput();
		input = fInput;
	}
	engine->device.renderInfo.camera.input = input;


	SDL_Event event;
	bool run = true;
	Uint32 currentTime = SDL_GetTicks(), lastTime = SDL_GetTicks();
	unsigned int rendered = 0;

	while (run) {
		currentTime = SDL_GetTicks();
		float deltaTime = float(currentTime - lastTime) / 1000.0f;

		while(SDL_PollEvent(&event)) {
			bool sdl_quit = event.type == SDL_QUIT;
			bool window_quit = event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE;
			if (sdl_quit || window_quit) run = false;
			else input->handleEvents(event);
		}

		for (GraphicsObject* obj: objs) obj->update(deltaTime);

		if (!renderLimit || rendered < maxRenderCount) {
			int64_t renderTime = measureExecTimeMicroseconds([&engine]() {
				engine->render();
			});
			float seconds = microsecondsToSeconds(renderTime);
			SDL_LogInfo(SDL_LOG_CATEGORY_SYSTEM, "Render Time: %f (~%i FPS)", seconds, int(1.0f / seconds));

			rendered++;
		} else if (rendered == maxRenderCount) {
			SDL_LogInfo(SDL_LOG_CATEGORY_SYSTEM, "Render finished!");
			engine->saveLatestImage(resultImagePath);
			break;
		}

		if (renderLimit) {
			SDL_Delay(100);
		} else {
			int sleepTime = currentTime + MS_PER_FRAME - SDL_GetTicks();
			if (sleepTime > 0) SDL_Delay(sleepTime);
		}
		lastTime = currentTime;
	}

	delete input;
	delete meshManager;
	delete engine;

	return 0;
}
