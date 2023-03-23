#include "path_tracer.h"

#define SURFACE_DISTANCE_OFFSET 0.01f


PathTracer::PathTracer()
:rng(new RandomGenerator()) {}

PathTracer::~PathTracer() {
	delete rng;
}

void PathTracer::parseInput(const InputEntry& inputEntry) {
	visionJumpCount = inputEntry.get<unsigned int>("visionJumpCount");
	raysPerPixel    = inputEntry.get<unsigned int>("raysPerPixel");
}

Vector3f PathTracer::renderPixel(const PixelRenderData& prd) const {
	Vector2f pixelCenter = Vector2f({(float) prd.pixel[0], (float) prd.pixel[1]}) + Vector2f({0.5f, 0.5f});
	Vector2f inUV = Vector2f({pixelCenter[0] / (float) prd.imageSize[0], pixelCenter[1] / (float) prd.imageSize[1]});
	Vector2f d = (2.0f * inUV) - Vector2f({1.0f, 1.0f});
	Vector3f target = cutVector(prd.projInverse * Vector4f({d[0], d[1], 1.0f, 1.0f})).normalize();
	Vector3f direction = cutVector(prd.viewInverse * expandVector(target, 0.0f)).normalize();

	Vector3f finalColor({0.0f, 0.0f, 0.0f});
	std::vector<HitPoint> visionPath(visionJumpCount);
	Ray startVisionRay(prd.origin, direction);
	for (unsigned int i = 0; i < raysPerPixel; ++i) {
		uint visionPathDepth = traceSinglePath(prd, visionPath, startVisionRay, 0, visionJumpCount);

		if (visionPathDepth == 0) continue;
		if (!visionPath[visionPathDepth - 1].lightHit) continue;

		finalColor += visionPath[visionPathDepth - 1].cumulativeColor;
	}
	
	finalColor *= 15.0f * 2.0f * M_PI * (1.0f / float(raysPerPixel));

	return finalColor;
}

size_t PathTracer::traceSinglePath(const PixelRenderData& prd, std::vector<HitPoint>& path, Ray ray, size_t startDepth, size_t maxDepth) const {
	Vector3f color = Vector3f({1.0f, 1.0f, 1.0f});
	size_t pathDepth = 0;

	Mesh::Vertex hitVertex;
	const GraphicsObject* obj;
	bool backfaceCulling = true;

	for (size_t i = startDepth; i < maxDepth; ++i) {
		if (!prd.scene->traceRay(ray, hitVertex, obj)) {
			break;
		} else {
			float ndotd = hitVertex.normal.dot(ray.direction);
			if (backfaceCulling && ndotd > 0.0f) {
				ray.origin = hitVertex.pos + SURFACE_DISTANCE_OFFSET * ray.direction;
				continue;
			}
			backfaceCulling = false;

			pathDepth = i + 1;
			float rayHandlingValue = rng->rand();

			Vector3f prevDirection = ray.direction;
			if (rayHandlingValue <= obj->diffuseThreshold) {
				ray.direction = rng->randomNormalDirection(hitVertex.normal);
			} else if (rayHandlingValue <= obj->reflectThreshold) {
				ray.direction = reflect(ray.direction, hitVertex.normal);
			} else if (rayHandlingValue <= obj->transparentThreshold) {
				ray.direction = customRefract(ray.direction, hitVertex.normal, obj->refractionIndex);
			}
			ray.origin = hitVertex.pos + SURFACE_DISTANCE_OFFSET * ray.direction;
			ray.update();

			if (rayHandlingValue <= obj->diffuseThreshold) {
				color *= obj->color * hitVertex.normal.dot(ray.direction);
				path[i].diffuse = true;
			} else {
				path[i].diffuse = false;
			}

			path[i].pos = hitVertex.pos;
			path[i].normal = hitVertex.normal;
			path[i].cumulativeColor = color;
			path[i].lightHit = false;

			if (obj->lightSource) {
				if (hitVertex.normal.dot(prevDirection) <= 0.0f) {
					path[i].lightHit = true;
					break;
				}
			}
		}
	}

	return pathDepth;
}
