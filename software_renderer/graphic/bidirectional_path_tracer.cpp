#include "bidirectional_path_tracer.h"

#define SURFACE_DISTANCE_OFFSET 0.01f


BidirectionalPathTracer::BidirectionalPathTracer()
:rng(new RandomGenerator()) {}

BidirectionalPathTracer::~BidirectionalPathTracer() {
	delete rng;
}

void BidirectionalPathTracer::parseInput(const InputEntry& inputEntry) {
	visionJumpCount = inputEntry.get<unsigned int>("visionJumpCount");
	lightJumpCount  = inputEntry.get<unsigned int>("lightJumpCount");
	maxDepth        = inputEntry.get<unsigned int>("maxDepth");
	raysPerPixel    = inputEntry.get<unsigned int>("raysPerPixel");
}

Vector3f BidirectionalPathTracer::renderPixel(const PixelRenderData& prd) const {
	Vector2f pixelCenter = Vector2f({(float) prd.pixel[0], (float) prd.pixel[1]}) + Vector2f({0.5f, 0.5f});
	Vector2f inUV = Vector2f({pixelCenter[0] / (float) prd.imageSize[0], pixelCenter[1] / (float) prd.imageSize[1]});
	Vector2f d = (2.0f * inUV) - Vector2f({1.0f, 1.0f});
	Vector3f target = cutVector(prd.projInverse * Vector4f({d[0], d[1], 1.0f, 1.0f})).normalize();
	Vector3f direction = cutVector(prd.viewInverse * expandVector(target, 0.0f)).normalize();

	Vector3f finalColor({0.0f, 0.0f, 0.0f});
	std::vector<HitPoint> visionPath(visionJumpCount);
	std::vector<HitPoint> lightPath(lightJumpCount);
	Ray startVisionRay(prd.origin, direction);
	for (unsigned int i = 0; i < raysPerPixel; ++i) {
		uint visionPathDepth = traceSinglePath(prd, visionPath, startVisionRay, 0, visionJumpCount, false);

		if (visionPathDepth == 0) continue;

		if (visionPath[visionPathDepth - 1].lightHit) {
			finalColor += visionPath[visionPathDepth - 1].cumulativeColor;
			continue;
		}

		LightSourcePoint lsp = getRandomLightSourcePoint(prd);
		Ray startLightRay(lsp.pos, rng->randomNormalDirection(lsp.normal));
		lightPath[0].pos = startLightRay.origin;
		lightPath[0].normal = lsp.normal;
		lightPath[0].cumulativeColor = lsp.color;
		lightPath[0].diffuse = true;
		uint lightPathDepth = traceSinglePath(prd, lightPath, startLightRay, 1, lightJumpCount, true);

		bool done = false;
		Vector3f color({0.0f, 0.0f, 0.0f});

		for (uint vi = 0; vi < visionPathDepth; ++vi) {
			for (uint li = 0; li < lightPathDepth; ++li) {
				if (vi + li > maxDepth) continue;

				Vector3f startPos = visionPath[vi].pos + SURFACE_DISTANCE_OFFSET * visionPath[vi].normal;
				Vector3f endPos = lightPath[li].pos + SURFACE_DISTANCE_OFFSET * lightPath[li].normal;

				bool occluded = prd.scene->isOccluded(startPos, endPos);
				if (!occluded) {
					Vector3f direction = endPos - startPos;
					direction.normalize();

					Vector3f visionColor = visionPath[vi].cumulativeColor;
					Vector3f lightColor = lightPath[li].cumulativeColor;

					color += visionColor * lightColor * direction.dot(visionPath[vi].normal);
					done = true;
				}
			}
		}

		// if (done) finalColor += color * (1.0f / float(visionPathDepth * lightPathDepth));
		if (done) finalColor += color * (1.0f / float(maxDepth));
	}
	
	finalColor *= 2.0f * M_PI * (1.0f / float(raysPerPixel));

	return finalColor;
}

size_t BidirectionalPathTracer::traceSinglePath(const PixelRenderData& prd, std::vector<HitPoint>& path, Ray ray, size_t startDepth, size_t maxDepth, bool isLightRay) const {
	Vector3f color = Vector3f({1.0f, 1.0f, 1.0f});
	size_t pathDepth = 0;

	Mesh::Vertex hitVertex;
	const GraphicsObject* obj;
	bool backfaceCulling = !isLightRay;

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
				Vector3f direction = isLightRay ? prevDirection : ray.direction;
				color *= obj->color * hitVertex.normal.dot(direction);
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

BidirectionalPathTracer::LightSourcePoint BidirectionalPathTracer::getRandomLightSourcePoint(const PixelRenderData& prd) const {
	size_t lightIndex = rng->rand() * float(prd.lightSources->size());
	GraphicsObject* lightSource = prd.lightSources->at(lightIndex);

	float sqrtr1 = sqrt(rng->rand());
	float r2 = rng->rand();
	Vector3f barycentricCoords({1.0f - sqrtr1, sqrtr1 * (1.0f - r2), sqrtr1 * r2});

	size_t ti = rng->rand() * float(lightSource->triangles.size());
	Triangle& t = lightSource->triangles[ti];

	LightSourcePoint lsp;

	lsp.pos = t.v0 * barycentricCoords[0] + t.v1 * barycentricCoords[1] + t.v2 * barycentricCoords[2];

	const Mesh::Vertex& vert0 = lightSource->vertices[t.indices[0]];
	const Mesh::Vertex& vert1 = lightSource->vertices[t.indices[1]];
	const Mesh::Vertex& vert2 = lightSource->vertices[t.indices[2]];
	lsp.normal = Vector3f({0.0f, 0.0f, 0.0f});
	lsp.normal += barycentricCoords[0] * vert0.normal;
	lsp.normal += barycentricCoords[1] * vert1.normal;
	lsp.normal += barycentricCoords[2] * vert2.normal;
	lsp.normal.normalize();

	lsp.color = lightSource->color;
	lsp.lightStrength = lightSource->lightStrength;

	return lsp;
}
