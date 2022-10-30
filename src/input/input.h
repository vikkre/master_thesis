#pragma once

#include <SDL2/SDL.h>

#include "../math/vector.h"


class Input {
	public:
		Input() {}
		virtual ~Input() {}

		virtual void handleEvents(const SDL_Event& event)=0;
		virtual void update(float /* deltaTime */) {}
		
		virtual Vector3f getPosition() const=0;
		virtual Vector3f getLookAt() const=0;
		virtual Vector3f getUp() const=0;
};
