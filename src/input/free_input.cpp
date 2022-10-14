#include "free_input.h"

#define MOVE_SPEED  1.0f
#define ANGLE_SPEED 0.01f

#define PHI_MIN       0.01f
#define PHI_MAX       M_PI - 0.01f
#define THETA_MIN    -M_PI
#define THETA_MAX     M_PI
#define THETA_CHANGE  M_PI * 2.0f


FreeInput::FreeInput()
:Input(), theta(M_PI), phi(M_PI_2),
position({22.0f, 0.0f, 0.0f}), checkMouseMotion(false) {}

FreeInput::~FreeInput() {}

void FreeInput::handleEvents(const SDL_Event& event) {
	if (event.type == SDL_KEYDOWN) {
		switch (event.key.keysym.sym) {
			case SDLK_w:      move(+1.0f,  0.0f);         break;
			case SDLK_s:      move(-1.0f,  0.0f);         break;
			case SDLK_a:      move( 0.0f, +1.0f);         break;
			case SDLK_d:      move( 0.0f, -1.0f);         break;
			case SDLK_LSHIFT: position[1] -= 1.0f; break;
			case SDLK_SPACE:  position[1] += 1.0f; break;
			case SDLK_ESCAPE: toggleMouse(false);  break;
		}

	} else if (event.type == SDL_MOUSEBUTTONDOWN) {
		toggleMouse(true);
	} else if (event.type == SDL_MOUSEMOTION) {
		if (checkMouseMotion) {
			theta += float(event.motion.xrel) * ANGLE_SPEED;
			phi   += float(event.motion.yrel) * ANGLE_SPEED;

			if (phi < PHI_MIN) phi = PHI_MIN;
			if (phi > PHI_MAX) phi = PHI_MAX;
			if (theta < THETA_MIN) theta += THETA_CHANGE;
			if (theta > THETA_MAX) theta -= THETA_CHANGE;
		}
	}
}

Vector3f FreeInput::getPosition() const {
	return position;
}

Vector3f FreeInput::getLookAt() const {
	return getLookDirection() + position;
}

Vector3f FreeInput::getUp() const {
	return Vector3f({0.0f, 1.0f, 0.0f});
}

Vector3f FreeInput::getLookDirection() const {
	return Vector3f({
		sin(phi) * cos(theta),
		cos(phi),
		sin(phi) * sin(theta)
	});
}

void FreeInput::toggleMouse(bool checkMouseMotion) {
	SDL_ShowCursor(checkMouseMotion ? SDL_DISABLE : SDL_ENABLE);
	SDL_SetRelativeMouseMode(checkMouseMotion ? SDL_TRUE : SDL_FALSE);
	this->checkMouseMotion = checkMouseMotion;
}

void FreeInput::move(float direction, float side) {
	Vector3f lookDirection = getLookDirection();
	lookDirection[1] = 0.0f;
	lookDirection.normalize();
	Vector3f sideDirection = Vector3f({lookDirection[2], 0.0f, -lookDirection[0]});
	Vector3f moveDirection = (direction * lookDirection) + (side * sideDirection);
	position += MOVE_SPEED * moveDirection;
}
