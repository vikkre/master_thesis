#pragma once

#include "input.h"


class FakeInput: public Input {
	public:
		FakeInput(Vector3f position);
		~FakeInput();

		virtual void handleEvents(const SDL_Event& event) override;
		virtual Vector3f getPosition() const override;

		Vector3f position;
};
