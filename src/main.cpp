#include <SDL2/SDL.h>
#include <iostream>
#include <map>
#include <algorithm>
#include <chrono>
#include <functional>
#include <filesystem>
#include <stdio.h>
#include <unistd.h>

#include "graphic/device.h"
#include "graphic/graphics_engine.h"
#include "graphic/mesh.h"
#include "graphic/graphics_object.h"
#include "graphic/helper/top_acceleration_structure_buffer.h"
#include "graphic/renderer/photon_mapper.h"
#include "graphic/renderer/unidirectional_path_tracer.h"
#include "graphic/renderer/shadow_tracer.h"
#include "graphic/renderer/majercik2019_renderer.h"
#include "graphic/renderer/bitterli2020_renderer.h"
#include "graphic/renderer/meta_renderer.h"
#include "graphic/renderer/phong_renderer.h"
#include "graphic/denoiser/gauss_denoiser.h"
#include "graphic/denoiser/median_denoiser.h"

#include "init_exception.h"
#include "mesh_manager.h"
#include "input_parser.h"

#include "free_input.h"

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
	if (name == "PhotonMapper")             return new PhotonMapper(device);
	if (name == "UnidirectionalPathTracer") return new UnidirectionalPathTracer(device);
	if (name == "ShadowTracer")             return new ShadowTracer(device);
	if (name == "Majercik2019")             return new Majercik2019(device);
	if (name == "Bitterli2020")             return new Bitterli2020(device);
	if (name == "MetaRenderer")             return new MetaRenderer(device);
	if (name == "PhongShader")              return new PhongRenderer(device);
	else return nullptr;
}

Denoiser* getDenoiser(const std::string& name, Device* device) {
	if      (name == "GaussDenoiser")  return new GaussDenoiser(device);
	else if (name == "MedianDenoiser") return new MedianDenoiser(device);
	else return nullptr;
}


void screenshot(GraphicsEngine* engine, const std::string& rendererName) {
	std::string path = std::string("out/") + rendererName + "_0.ppm";
	for (unsigned int i = 1; std::filesystem::exists(path); ++i) {
		path = std::string("out/") + rendererName + "_" + std::to_string(i) + ".ppm";
	}
	engine->saveLatestImage(path);
	std::cout << "Saved screenshot " << path << std::endl;
}

int main(int argc, char* argv[]) {
	if (argc != 5 && argc != 6 && argc != 8) {
		std::cout << "Error: wrong paramter count!" << std::endl;
		std::cout << "Usage: RayTrace renderer scene image_width image_height [camera] [resultimage renderrounds]" << std::endl;
		return -1;
	}

	const std::string execpath = argv[0];
	const std::string basepath = execpath.substr(0, execpath.size() - sizeof("RayTrace") + 1);

	std::string rendererPath = argv[1];
	std::string scenePath = argv[2];
	Vector2i windowSize = Vector2i({std::atoi(argv[3]), std::atoi(argv[4])});
	std::string cameraFilePath;
	std::string resultImagePath;
	unsigned int maxRenderCount;

	bool hasCamera = argc == 6 || argc == 8;
	if (hasCamera) {
		cameraFilePath = argv[5];
	}

	bool renderLimit = argc == 8;
	if (renderLimit) {
		resultImagePath = argv[6];
		maxRenderCount = std::atoi(argv[7]);
	}

	size_t start = rendererPath.find_last_of('/') + 1;
	size_t finish = rendererPath.find_last_of('.') - start;
	std::string rendererName = rendererPath.substr(start, finish);


	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		InitException("SDL_Init", SDL_GetError());
	}
	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);

	GraphicsEngine* engine = new GraphicsEngine(basepath);
	engine->windowSize = windowSize;
	engine->init();

	MeshManager* meshManager = new MeshManager(&engine->device, basepath);

	meshManager->createObjectsFromFile(scenePath);

	InputParser rendererParser(rendererPath);
	rendererParser.parse();

	engine->renderer = getRenderer(rendererParser.getInputEntry(0).name, &engine->device);
	engine->renderer->parseInput(rendererParser.getInputEntry(0));
	std::vector<GraphicsObject*> objs = meshManager->getCreatedObjects();
	engine->renderer->passObjects(objs);
	std::vector<GraphicsObject*> lightSources = meshManager->getCreatedLightSources();
	engine->renderer->passLightSources(lightSources);

	for (unsigned int i = 1; i < rendererParser.size(); ++i) {
		const InputEntry& inputEntry = rendererParser.getInputEntry(i);
		Denoiser* denoiser = getDenoiser(inputEntry.name, &engine->device);
		denoiser->parseInput(inputEntry);
		engine->denoisers.push_back(denoiser);
	}

	engine->initTlas();

	FreeInput* input = new FreeInput(renderLimit);
	if (hasCamera) {
		InputParser cameraParser(cameraFilePath);
		cameraParser.parse();
		input->parseInput(cameraParser.getInputEntry(0));
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
			bool button_screenshot = event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_i;

			if (sdl_quit || window_quit) run = false;
			else if (button_screenshot) screenshot(engine, rendererName);
			else input->handleEvents(event);
		}

		input->update(deltaTime);

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
