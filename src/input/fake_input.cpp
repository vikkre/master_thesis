#include "fake_input.h"


FakeInput::FakeInput(Vector3f position)
:Input(), position(position) {}

FakeInput::~FakeInput() {}

void FakeInput::handleEvents(const SDL_Event& /* event */) {}

Vector3f FakeInput::getPosition() const {
	return position;
}
