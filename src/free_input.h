#pragma once

#include <SDL2/SDL.h>

#include "input_parser.h"

#include "math/matrix.h"
#include "math/rotation.h"


class FreeInput {
	public:
		FreeInput(bool disableControl);
		~FreeInput();

		void handleEvents(const SDL_Event& event);
		void update(float deltaTime);

		Vector3f getPosition() const;
		Vector3f getLookAt() const;
		Vector3f getUp() const;

		Vector3f getLookDirection() const;
		void toggleMouse(bool escape);
		void move(float direction, float side, float deltaTime);
		void parseInput(const InputEntry& inputEntry);

		float theta;
		float phi;

		Vector3f position;

	private:
		bool disableControl;
		bool checkMouseMotion;

		bool forward, backward;
		bool left, right;
		bool up, down;
};
