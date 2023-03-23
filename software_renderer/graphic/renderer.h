#pragma once

#include "../math/vector.h"
#include "../math/matrix.h"
#include "../input_parser.h"
#include "scene.h"


class Renderer {
	public:
		struct PixelRenderData {
			const Scene* scene;
			const std::vector<GraphicsObject*>* objects;
			const std::vector<GraphicsObject*>* lightSources;

			Vector2u imageSize;
			Vector2u pixel;

			Vector3f origin;
			Matrix4f viewInverse;
			Matrix4f projInverse;
		};
		Renderer() {}
		virtual ~Renderer() {}

		virtual void parseInput(const InputEntry& inputEntry)=0;
		virtual Vector3f renderPixel(const PixelRenderData& prd) const=0;
};
