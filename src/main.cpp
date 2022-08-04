#include <SDL2/SDL.h>
#include <iostream>
#include <map>
#include <chrono>
#include <functional>

#include "graphic/device.h"
#include "graphic/graphics_engine.h"
#include "graphic/mesh.h"
#include "graphic/graphics_object.h"
#include "graphic/helper/top_acceleration_structure_buffer.h"

#include "init_exception.h"
#include "stl_loader.h"
#include "obj_loader.h"
#include "input.h"

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

void rayTracingRoom(GraphicsEngine* engine, std::vector<Mesh*>& meshes, std::vector<GraphicsObject*>& objs, float reflect) {
	ObjLoader blockLoader;
	blockLoader.load("block.obj");
	Mesh* block = blockLoader.get_mesh(&engine->device);
	meshes.push_back(block);

	GraphicsObject* back = new GraphicsObject(&engine->device, block, Vector3f({-5.0f, 0.0f, 0.0f}));
	back->scale = Vector3f({0.1f, 5.0f, 5.0f});
	objs.push_back(back);

	GraphicsObject* top = new GraphicsObject(&engine->device, block, Vector3f({0.0f, 5.0f, 0.0f}));
	top->scale = Vector3f({5.0f, 0.1f, 5.0f});
	objs.push_back(top);

	GraphicsObject* bottom = new GraphicsObject(&engine->device, block, Vector3f({0.0f, -5.0f, 0.0f}));
	bottom->scale = Vector3f({5.0f, 0.1f, 5.0f});
	objs.push_back(bottom);

	GraphicsObject* red = new GraphicsObject(&engine->device, block, Vector3f({0.0f, 0.0f, 5.0f}));
	red->color = Vector3f({1.0f, 0.0f, 0.0f});
	red->scale = Vector3f({5.0f, 5.0f, 0.1f});
	objs.push_back(red);
	
	GraphicsObject* green = new GraphicsObject(&engine->device, block, Vector3f({0.0f, 0.0f, -5.0f}));
	green->color = Vector3f({0.0f, 1.0f, 0.0f});
	green->scale = Vector3f({5.0f, 5.0f, 0.1f});
	objs.push_back(green);
}


void blocksAndBall(GraphicsEngine* engine, std::vector<Mesh*>& meshes, std::vector<GraphicsObject*>& objs, float reflect) {
	ObjLoader blockLoader;
	blockLoader.load("block.obj");
	Mesh* block = blockLoader.get_mesh(&engine->device);
	meshes.push_back(block);

	ObjLoader ballLoader;
	ballLoader.load("ball.obj");
	Mesh* ball = ballLoader.get_mesh(&engine->device);
	meshes.push_back(ball);

	GraphicsObject* white = new GraphicsObject(&engine->device, ball, Vector3f({0.0f, 0.0f, 0.0f}));
	white->color = Vector3f({1.0f, 1.0f, 1.0f});
	objs.push_back(white);
	white->rtData.reflect = reflect;
	
	GraphicsObject* red = new GraphicsObject(&engine->device, block, Vector3f({0.0f, 0.0f, 3.0f}));
	red->color = Vector3f({1.0f, 0.0f, 0.0f});
	objs.push_back(red);
	
	GraphicsObject* green = new GraphicsObject(&engine->device, block, Vector3f({-3.0f, 0.0f, 0.0f}));
	green->color = Vector3f({0.0f, 1.0f, 0.0f});
	objs.push_back(green);
	
	GraphicsObject* blue = new GraphicsObject(&engine->device, block, Vector3f({0.0f, 0.0f, -3.0f}));
	blue->color = Vector3f({0.0f, 0.0f, 1.0f});
	objs.push_back(blue);
	
	GraphicsObject* black = new GraphicsObject(&engine->device, block, Vector3f({3.0f, 0.0f, 0.0f}));
	black->color = Vector3f({0.1f, 0.1f, 0.1f});
	objs.push_back(black);
}

void teeth(GraphicsEngine* engine, std::vector<Mesh*>& meshes, std::vector<GraphicsObject*>& objs, float reflect) {
	STLLoader loader;
	loader.load("../res/stl/Upper.stl");
	Mesh* upper_mesh = loader.get_mesh(&engine->device);
	meshes.push_back(upper_mesh);

	loader.load("../res/stl/Lower.stl");
	Mesh* lower_mesh = loader.get_mesh(&engine->device);
	meshes.push_back(lower_mesh);

	GraphicsObject* upper = new GraphicsObject(&engine->device, upper_mesh, Vector3f({0.0f, 0.0f, 0.0f}));
	upper->color = Vector3f({1.0f, 0.0f, 0.0f});
	objs.push_back(upper);

	GraphicsObject* lower = new GraphicsObject(&engine->device, lower_mesh, Vector3f({0.0f, 0.0f, 0.0f}));
	lower->color = Vector3f({0.0f, 1.0f, 0.0f});
	objs.push_back(lower);
	lower->rtData.reflect = reflect;
}


int main() {
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		InitException("SDL_Init", SDL_GetError());
	}
	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);

	GraphicsEngine* engine = new GraphicsEngine();
	engine->init();

	// engine->device.renderInfo.backgroundColor = Vector3f({0.5f, 0.5f, 0.5f});
	engine->device.renderInfo.backgroundColor = Vector3f({0.0f, 0.0f, 0.0f});
	engine->device.renderInfo.camera.position = Vector3f({0.0f, 0.0f, 0.0f});
	engine->device.renderInfo.camera.lookAt   = Vector3f({0.0f, 0.0f, 0.0f});
	engine->device.renderInfo.lightPosition   = Vector3f({0.0f, 4.9f, 0.0f});

	std::vector<Mesh*> meshes;
	std::vector<GraphicsObject*> objs;

	rayTracingRoom(engine, meshes, objs, 0.0f);
	// blocksAndBall(engine, meshes, objs, 0.0f);
	// blocksAndBall(engine, meshes, objs, 1.0f);
	// teeth(engine, meshes, objs, 0.0f);
	// teeth(engine, meshes, objs, 1.0f);

	for (GraphicsObject* obj: objs) {
		engine->renderer.objects.push_back(obj);
	}
	engine->initTlas();

	Input* input = new Input();
	input->r = 15.0f;
	input->phi = M_PI_2;


	SDL_Event event;
	bool run = true;
	Uint32 currentTime = SDL_GetTicks(), lastTime = SDL_GetTicks();
	// float lightAngle = 0.0f;

	bool rendered = false;
	while (run) {
		currentTime = SDL_GetTicks();
    float deltaTime = float(currentTime - lastTime) / 1000.0f;
    SDL_LogVerbose(SDL_LOG_CATEGORY_SYSTEM, "Current delta time: %f", deltaTime);

		while(SDL_PollEvent(&event)) {
			if (event.type == SDL_WINDOWEVENT) {
				switch (event.window.event) {
					case SDL_WINDOWEVENT_CLOSE: run = false; break;
				}
			} else {
				switch (event.type) {
					case SDL_QUIT: run = false; break;
					default: input->handleEvents(event); break;
				}
			}
		}

		engine->device.renderInfo.camera.position = input->getPosition();

		// lightAngle += M_PI_2 * deltaTime;
		// lightAngle = 1.0;
		// float lightY = engine->device.renderInfo.lightPosition[1];
		// engine->device.renderInfo.lightPosition = Vector3f({cos(lightAngle) * 10.0f, lightY, sin(lightAngle) * 10.0f});

		if (!rendered) {
			engine->render();
			rendered = true;
		}

		// int64_t renderTime = measureExecTimeMicroseconds([&engine]() {
		// 	engine->render();
		// });
		// SDL_LogInfo(SDL_LOG_CATEGORY_SYSTEM, "Render Time: %f", microsecondsToSeconds(renderTime));

		int sleepTime = currentTime + MS_PER_FRAME - SDL_GetTicks();
    SDL_LogVerbose(SDL_LOG_CATEGORY_SYSTEM, "Sleep time: %i", sleepTime);
		if (sleepTime > 0) SDL_Delay(sleepTime);
		lastTime = currentTime;
	}

	delete input;
	for (GraphicsObject* obj: objs) delete obj;
	for (Mesh* mesh: meshes) delete mesh;

	delete engine;

	return 0;
}
