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
#include "graphic/renderer/monte_carlo_renderer.h"
#include "graphic/renderer/praktikums_renderer.h"
#include "graphic/denoiser/gauss_denoiser.h"
#include "graphic/denoiser/median_denoiser.h"

#include "init_exception.h"
#include "mesh_manager.h"
#include "input_parser.h"

#include "math/vector.h"
#include "math/matrix.h"
#include "math/rotation.h"


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
	if (name == "MonteCarloRenderer") return new MonteCarloRenderer(device);
	else return nullptr;
}

Denoiser* getDenoiser(const std::string& name, Device* device) {
	if      (name == "GaussDenoiser")  return new GaussDenoiser(device);
	else if (name == "MedianDenoiser") return new MedianDenoiser(device);
	else return nullptr;
}


int main(int /* argc */, char* argv[]) {
	const std::string execpath = argv[0];
	const std::string basepath = execpath.substr(0, execpath.size() - sizeof("RayTrace") + 1);

	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		InitException("SDL_Init", SDL_GetError());
	}
	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);

	GraphicsEngine* engine = new GraphicsEngine(basepath);
	engine->init();

	engine->device.renderInfo.lightPosition = Vector3f({0.0f, 4.5f, 0.0f});
	engine->device.renderInfo.camera.position = Vector3f({22.0f, 0.0f, 0.0f});

	MeshManager* meshManager = new MeshManager(&engine->device, basepath);

	// meshManager->createObjectsFromFile("../res/scene/cornell_box.scene");
	meshManager->createObjectsFromFile("../res/scene/cornell_box_with_blocks.scene");

	InputParser parser(basepath + "../res/renderer/full_monte_carlo.renderer");
	parser.parse();

	engine->renderer = getRenderer(parser.getInputEntry(0).name, &engine->device);
	engine->renderer->parseInput(parser.getInputEntry(0));
	engine->renderer->passObjects(meshManager->getCreatedObjects());

	for (unsigned int i = 1; i < parser.size(); ++i) {
		const InputEntry& inputEntry = parser.getInputEntry(i);
		Denoiser* denoiser = getDenoiser(inputEntry.name, &engine->device);
		denoiser->parseInput(inputEntry);
		engine->denoisers.push_back(denoiser);
	}

	// engine->renderer->objects = meshManager->getCreatedObjects();
	// engine->denoisers.push_back(gaussDenoiser);
	// engine->denoisers.push_back(medianDenoiser);
	engine->initTlas();

	// MonteCarloRenderer* monteCarloRenderer = new MonteCarloRenderer(&engine->device);

	// monteCarloRenderer->renderSettings.backgroundColor = Vector3f({0.0f, 0.0f, 0.0f});
	// monteCarloRenderer->renderSettings.lightPosition = Vector3f({0.0f, 4.5f, 0.0f});
	// monteCarloRenderer->renderSettings.lightRayCount = 250;
	// monteCarloRenderer->renderSettings.lightJumpCount = 5;
	// monteCarloRenderer->renderSettings.visionJumpCount = 5;
	// monteCarloRenderer->renderSettings.collectionDistance = 0.4f;
	// monteCarloRenderer->renderSettings.visionRayPerPixelCount = 30;
	// monteCarloRenderer->renderSettings.collectionDistanceShrinkFactor = 5.0f;
	// monteCarloRenderer->renderSettings.lightCollectionCount = 10;
	// monteCarloRenderer->renderSettings.useCountLightCollecton = false;

	// monteCarloRenderer->objects = meshManager->getCreatedObjects();

	// GaussDenoiser* gaussDenoiser = new GaussDenoiser(&engine->device);
	// gaussDenoiser->settings.kernelSize = 7;
	// gaussDenoiser->settings.sigma = 0.8f;

	// MedianDenoiser* medianDenoiser = new MedianDenoiser(&engine->device);
	// medianDenoiser->settings.kernelSize = 7;

	// engine->renderer = monteCarloRenderer;
	// engine->denoisers.push_back(gaussDenoiser);
	// engine->denoisers.push_back(medianDenoiser);
	// engine->initTlas();


	SDL_Event event;
	bool run = true;

	const unsigned int RENDER_MAX = 1;
	unsigned int rendered = 0;

	while (run) {
		while(SDL_PollEvent(&event)) {
			bool sdl_quit = event.type == SDL_QUIT;
			bool window_quit = event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE;
			if (sdl_quit || window_quit) run = false;
		}

		if (rendered < RENDER_MAX) {
			int64_t renderTime = measureExecTimeMicroseconds([&engine]() {
				engine->render();
			});
			float seconds = microsecondsToSeconds(renderTime);
			SDL_LogInfo(SDL_LOG_CATEGORY_SYSTEM, "Render Time: %f (~%i FPS)", seconds, int(1.0f / seconds));

			rendered++;
		} else if (rendered == RENDER_MAX) {
			SDL_LogInfo(SDL_LOG_CATEGORY_SYSTEM, "Render finished!");

			rendered++;
		}

		SDL_Delay(100);
	}

	// delete medianDenoiser;
	// delete gaussDenoiser;
	// delete monteCarloRenderer;
	delete meshManager;
	delete engine;

	return 0;
}
