#include "fake_input.h"


FakeInput::FakeInput(Vector3f position)
:Input(), position(position), lookAt({0.0f, 0.0f, 0.0f}), up({0.0f, 1.0f, 0.0f}) {}

FakeInput::~FakeInput() {}

void FakeInput::handleEvents(const SDL_Event& /* event */) {}

Vector3f FakeInput::getPosition() const {
	return position;
}

Vector3f FakeInput::getLookAt() const {
	return lookAt;
}

Vector3f FakeInput::getUp() const {
	return up;
}
