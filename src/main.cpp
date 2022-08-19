#include <SDL2/SDL.h>
#include <iostream>
#include <map>
#include <algorithm>
#include <chrono>
#include <functional>

#include "graphic/device.h"
#include "graphic/graphics_engine.h"
#include "graphic/mesh.h"
#include "graphic/graphics_object.h"
#include "graphic/helper/top_acceleration_structure_buffer.h"

#include "init_exception.h"
#include "mesh_manager.h"

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


int main() {
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		InitException("SDL_Init", SDL_GetError());
	}
	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);

	GraphicsEngine* engine = new GraphicsEngine();
	engine->init();

	engine->device.renderInfo.lightPosition = Vector3f({0.0f, 4.5f, 0.0f});
	engine->device.renderInfo.camera.position = Vector3f({22.0f, 0.0f, 0.0f});

	MeshManager* meshManager = new MeshManager(&engine->device);
	meshManager->init();

	meshManager->createCornellBox();
	meshManager->createCornellBoxBlocks(0.0f);

	engine->renderer.objects = meshManager->getCreatedObjects();
	engine->initTlas();


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

	delete meshManager;
	delete engine;

	return 0;
}
