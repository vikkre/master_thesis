#pragma once

#include "input.h"

#include "../math/matrix.h"
#include "../math/rotation.h"


class SphericalInput: public Input {
	public:
		SphericalInput();
		~SphericalInput();

		virtual void handleEvents(const SDL_Event& event) override;

		virtual Vector3f getPosition() const override;
		virtual Vector3f getLookAt() const override;
		virtual Vector3f getUp() const override;

		void toggleMouse(bool escape);

		float r;
		float theta;
		float phi;

		Vector3f lookAt;

	private:
		bool checkMouseMotion;
};
