#include "input.h"

#define R_SPEED      2.5f
#define ANGLE_SPEED 0.01f

#define PHI_MIN       0.01f
#define PHI_MAX       M_PI - 0.01f
#define THETA_MIN    -M_PI
#define THETA_MAX     M_PI
#define THETA_CHANGE  M_PI * 2.0f


Input::Input()
:r(1.0f), theta(0.0f), phi(M_PI_4), checkMouseMotion(false) {}

void Input::handleEvents(const SDL_Event& event) {
	if (event.type == SDL_KEYDOWN) {
		switch (event.key.keysym.sym) {
			case SDLK_w:      r -= R_SPEED;       break;
			case SDLK_s:      r += R_SPEED;       break;
			case SDLK_ESCAPE: toggleMouse(false); break;
		}

	} else if (event.type == SDL_MOUSEBUTTONDOWN) {
		toggleMouse(true);
	} else if (event.type == SDL_MOUSEMOTION) {
		if (checkMouseMotion) {
			theta += float(event.motion.xrel) * ANGLE_SPEED;
			phi   -= float(event.motion.yrel) * ANGLE_SPEED;

			if (phi < PHI_MIN) phi = PHI_MIN;
			if (phi > PHI_MAX) phi = PHI_MAX;
			if (theta < THETA_MIN) theta += THETA_CHANGE;
			if (theta > THETA_MAX) theta -= THETA_CHANGE;
		}
	}
}

void Input::toggleMouse(bool checkMouseMotion) {
	SDL_ShowCursor(checkMouseMotion ? SDL_DISABLE : SDL_ENABLE);
	SDL_SetRelativeMouseMode(checkMouseMotion ? SDL_TRUE : SDL_FALSE);
	this->checkMouseMotion = checkMouseMotion;
}

Vector3f Input::getPosition() const {
	return Vector3f({
		r * sin(phi) * cos(theta),
		r * cos(phi),
		r * sin(phi) * sin(theta),
	});
}
