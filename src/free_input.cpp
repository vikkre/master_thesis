#include "free_input.h"

#define MOVE_SPEED  7.0f
#define ANGLE_SPEED 0.01f

#define PHI_MIN       0.01f
#define PHI_MAX       M_PI - 0.01f
#define THETA_MIN    -M_PI
#define THETA_MAX     M_PI
#define THETA_CHANGE  M_PI * 2.0f


FreeInput::FreeInput(bool disableControl)
:theta(M_PI), phi(M_PI_2),
position({5.0f, 0.0f, 0.0f}), disableControl(disableControl), checkMouseMotion(false),
forward(false), backward(false), left(false), right(false), up(false), down(false) {}

FreeInput::~FreeInput() {}

void FreeInput::handleEvents(const SDL_Event& event) {
	if (disableControl) return;

	if (event.type == SDL_KEYDOWN) {
		switch (event.key.keysym.sym) {
			case SDLK_w:      forward  = true; break;
			case SDLK_s:      backward = true; break;
			case SDLK_a:      left     = true; break;
			case SDLK_d:      right    = true; break;
			case SDLK_SPACE:  up       = true; break;
			case SDLK_LSHIFT: down     = true; break;

			case SDLK_ESCAPE: toggleMouse(false); break;
		}

	} else if (event.type == SDL_KEYUP) {
		switch (event.key.keysym.sym) {
			case SDLK_w:      forward  = false; break;
			case SDLK_s:      backward = false; break;
			case SDLK_a:      left     = false; break;
			case SDLK_d:      right    = false; break;
			case SDLK_SPACE:  up       = false; break;
			case SDLK_LSHIFT: down     = false; break;
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

void FreeInput::update(float deltaTime) {
	if (disableControl) return;

	if (forward)  move(+1.0f,  0.0f, deltaTime);
	if (backward) move(-1.0f,  0.0f, deltaTime);
	if (left)     move( 0.0f, +1.0f, deltaTime);
	if (right)    move( 0.0f, -1.0f, deltaTime);

	if (up)   position[1] += MOVE_SPEED * deltaTime;
	if (down) position[1] -= MOVE_SPEED * deltaTime;
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

void FreeInput::move(float direction, float side, float deltaTime) {
	Vector3f lookDirection = getLookDirection();
	lookDirection[1] = 0.0f;
	lookDirection.normalize();
	Vector3f sideDirection = Vector3f({lookDirection[2], 0.0f, -lookDirection[0]});
	Vector3f moveDirection = (direction * lookDirection) + (side * sideDirection);
	position += MOVE_SPEED * deltaTime * moveDirection;
}

void FreeInput::parseInput(const InputEntry& inputEntry) {
	position = inputEntry.getVector<3, float>("position");
	theta = inputEntry.get<float>("angle", 0);
	phi = inputEntry.get<float>("angle", 1);

	theta *= M_PI;
	phi = phi * -M_PI_2 + M_PI_2;

	if (phi < PHI_MIN) phi = PHI_MIN;
	if (phi > PHI_MAX) phi = PHI_MAX;
	if (theta < THETA_MIN) theta += THETA_CHANGE;
	if (theta > THETA_MAX) theta -= THETA_CHANGE;
}
