#pragma once

#include "input_parser.h"

#include "math/vector.h"
#include "math/matrix.h"


class Camera {
	public:
		Camera();
		~Camera();

		void parseInput(const InputEntry& inputEntry);
		Matrix4f getViewMatrix() const;
		Matrix4f getProjectionMatrix(float aspectRatio) const;

		float theta;
		float phi;

		Vector3f position;

		float fov;
		float nearZ;
		float farZ;
		bool invertY;
};
