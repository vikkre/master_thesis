#pragma once

#include <vector>

#include "renderer.h"
#include "../math/random.h"


class PathTracer: public Renderer {
	public:
		PathTracer();
		~PathTracer();

		virtual void parseInput(const InputEntry& inputEntry) override;
		virtual Vector3f renderPixel(const PixelRenderData& prd) const override;
	
	private:
		struct HitPoint {
			Vector3f pos;
			Vector3f normal;
			Vector3f cumulativeColor;
			bool diffuse;
			bool lightHit;
		};

		RandomGenerator* rng;

		unsigned int visionJumpCount;
		unsigned int raysPerPixel;

		size_t traceSinglePath(const PixelRenderData& prd, std::vector<HitPoint>& path, Ray ray, size_t startDepth, size_t maxDepth) const;
};
