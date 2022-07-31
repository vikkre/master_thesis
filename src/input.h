#pragma once

#include <SDL2/SDL.h>

#include "math/vector.h"
#include "math/matrix.h"
#include "math/rotation.h"


class Input {
	public:
		Input();

		void handleEvents(const SDL_Event& event);
		void toggleMouse(bool escape);
		Vector3f getPosition() const;

		float r;
		float theta;
		float phi;

	private:
		bool checkMouseMotion;
};
