#pragma once

#include "input.h"

#include "../math/matrix.h"
#include "../math/rotation.h"


class RealInput: public Input {
	public:
		RealInput();
		~RealInput();

		virtual void handleEvents(const SDL_Event& event) override;
		virtual Vector3f getPosition() const override;

		void toggleMouse(bool escape);

		float r;
		float theta;
		float phi;

	private:
		bool checkMouseMotion;
};
