#pragma once

#include "input.h"

#include "../math/matrix.h"
#include "../math/rotation.h"


class FreeInput: public Input {
	public:
		FreeInput();
		~FreeInput();

		virtual void handleEvents(const SDL_Event& event) override;
		virtual void update(float deltaTime) override;

		virtual Vector3f getPosition() const override;
		virtual Vector3f getLookAt() const override;
		virtual Vector3f getUp() const override;

		Vector3f getLookDirection() const;
		void toggleMouse(bool escape);
		void move(float direction, float side, float deltaTime);

		float theta;
		float phi;

		Vector3f position;

	private:
		bool checkMouseMotion;

		bool forward, backward;
		bool left, right;
		bool up, down;
};
