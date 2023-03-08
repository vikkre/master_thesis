#include "graphics_engine.h"

#define SURFACE_DISTANCE_OFFSET 0.01f


void threadRender(GraphicsEngine* graphicsEngine, const Matrix4f& viewInverse, const Matrix4f& projInverse, const Vector3f& origin) {
	graphicsEngine->render(viewInverse, projInverse, origin);
}


GraphicsEngine::GraphicsEngine()
:imageSize(), image(), camera(nullptr), objects(), lightSources(), scene(), rng() {}

GraphicsEngine::~GraphicsEngine() {}

void GraphicsEngine::parseInput(const InputEntry& inputEntry) {
	raysPerPixel = inputEntry.get<unsigned int>("raysPerPixel");
	visionJumpCount = inputEntry.get<unsigned int>("visionJumpCount");
	lightJumpCount = inputEntry.get<unsigned int>("lightJumpCount");
	threadCount = inputEntry.get<unsigned int>("threadCount");
}

void GraphicsEngine::init() {
	for (GraphicsObject* obj: objects) {
		obj->init();
		scene.addObject(obj);
	}
	scene.init();
	image.resize(imageSize[0] * imageSize[1] * 3);
}

void GraphicsEngine::saveImage(const std::string path) {
	std::ofstream file(path, std::ios::out | std::ios::binary);

	constexpr unsigned int sizeformat = 255;
	file << "P6\n" << imageSize[0] << "\n" << imageSize[1] << "\n" << sizeformat << "\n";
	file.write(image.data(), image.size());

	file.close();
}

void GraphicsEngine::render() {
	Matrix4f viewInverse = camera->getViewMatrix().inverseMatrix();
	Matrix4f projInverse = camera->getProjectionMatrix(float(imageSize[0]) / float(imageSize[1])).inverseMatrix();

	Vector3f origin = cutVector(viewInverse * Vector4f({0.0f, 0.0f, 0.0f, 1.0f}));

	pixelCounter = 0;
	std::vector<std::thread> threads;
	threads.reserve(threadCount);

	for (unsigned int t = 0; t < threadCount; ++t) {
		threads.push_back(std::thread(threadRender, this, viewInverse, projInverse, origin));
	}

	for (std::thread& th: threads) th.join();
}

void GraphicsEngine::render(const Matrix4f& viewInverse, const Matrix4f& projInverse, const Vector3f& origin) {
	uint32_t fullSize = imageSize[0] * imageSize[1];
	uint32_t fullStep = fullSize / 100;
	while (true) {
		uint32_t currentPixel = pixelCounter.fetch_add(1);

		if (currentPixel >= fullSize) {
			if (currentPixel == fullSize) std::cout << "100% done" << std::endl;
			break;
		}

		if (currentPixel % fullStep == 0 && currentPixel != 0) {
			unsigned int done = (unsigned int)(100.0f * (float(currentPixel) / float(fullSize)));
			std::cout << done << "% done" << std::endl;
		}

		unsigned int x = currentPixel % imageSize[0];
		unsigned int y = currentPixel / imageSize[0];
		renderPixel(x, y, viewInverse, projInverse, origin);
	}
}

void GraphicsEngine::renderPixel(unsigned int x, unsigned int y, const Matrix4f& viewInverse, const Matrix4f& projInverse, const Vector3f& origin) {
	Vector2f pixelCenter = Vector2f({x + 0.5f, y + 0.5f});
	Vector2f inUV = Vector2f({pixelCenter[0] / (float) imageSize[0], pixelCenter[1] / (float) imageSize[1]});
	Vector2f d = (2.0f * inUV) - Vector2f({1.0f, 1.0f});
	Vector3f target = cutVector(projInverse * Vector4f({d[0], d[1], 1.0f, 1.0f})).normalize();
	Vector3f direction = cutVector(viewInverse * expandVector(target, 0.0f)).normalize();

	Vector3f finalColor;
	std::vector<HitPoint> visionPath(visionJumpCount);
	std::vector<HitPoint> lightPath(lightJumpCount);
	Ray startVisionRay(origin, direction);
	for (unsigned int i = 0; i < raysPerPixel; ++i) {
		uint visionPathDepth = traceSinglePath(startVisionRay, visionPath, visionJumpCount, 0, true);

		if (visionPathDepth == 0) continue;

		if (visionPath[visionPathDepth - 1].lightHit) {
			finalColor += visionPath[visionPathDepth - 1].cumulativeColor;
			continue;
		}

		LightSourcePoint lsp = getRandomLightSourcePoint();
		Ray startLightRay(lsp.pos, rng.randomNormalDirection(lsp.normal));
		lightPath[0].pos = startLightRay.origin;
		lightPath[0].cumulativeColor = lsp.color;
		lightPath[0].diffuse = true;
		uint lightPathDepth = traceSinglePath(startLightRay, lightPath, lightJumpCount, 1, false);

		bool done = false;
		Vector3f color({0.0f, 0.0f, 0.0f});

		for (uint vi = 0; vi < visionPathDepth; ++vi) {
			for (uint li = 0; li < lightPathDepth; ++li) {
				Vector3f startPos = visionPath[vi].pos + SURFACE_DISTANCE_OFFSET * visionPath[vi].normal;
				Vector3f endPos = lightPath[li].pos + SURFACE_DISTANCE_OFFSET * lightPath[li].normal;
				bool occluded = scene.isOccluded(startPos, endPos);
				if (!occluded) {
					Vector3f direction = endPos - startPos;
					direction.normalize();
					Vector3f normal = visionPath[vi].normal;

					Vector3f visionColor = visionPath[vi].cumulativeColor;
					Vector3f lightColor = lightPath[li].cumulativeColor;
					color += visionColor * lightColor * direction.dot(normal);
					done = true;
				}
			}
		}

		if (done) finalColor += color / float(visionPathDepth * lightPathDepth);
	}
	
	finalColor *= (2.0f * M_PI) / float(raysPerPixel);

	for (unsigned int i = 0; i < 3; ++i)
		finalColor[i] = std::clamp(finalColor[i], 0.0f, 1.0f);

	unsigned int index = (x + y * imageSize[0]) * 3;
	for (unsigned int i = 0; i < 3; ++i)
		image[index + i] = (char) (finalColor[i] * 255.0f);
}

size_t GraphicsEngine::traceSinglePath(Ray ray, std::vector<HitPoint>& path, size_t maxDepth, size_t startDepth, bool toLight) {
	Vector3f color = Vector3f({1.0f, 1.0f, 1.0f});
	size_t pathDepth = 0;

	Mesh::Vertex hitVertex;
	const GraphicsObject* obj;
	bool backfaceCulling = toLight;

	for (uint i = startDepth; i < maxDepth; ++i) {
		if (!scene.traceRay(ray, hitVertex, obj)) {
			break;
		} else {
			float ndotd = hitVertex.normal.dot(ray.direction);
			if (backfaceCulling && ndotd > 0.0f) {
				ray.origin = hitVertex.pos + SURFACE_DISTANCE_OFFSET * ray.direction;
				continue;
			}
			backfaceCulling = false;

			pathDepth = i + 1;
			float rayHandlingValue = rng.rand();

			Vector3f prevDirection = ray.direction;
			if (rayHandlingValue <= obj->diffuseThreshold) {
				ray.direction = rng.randomNormalDirection(hitVertex.normal);
			} else if (rayHandlingValue <= obj->reflectThreshold) {
				ray.direction = reflect(ray.direction, hitVertex.normal);
			} else if (rayHandlingValue <= obj->transparentThreshold) {
				ray.direction = customRefract(ray.direction, hitVertex.normal, obj->refractionIndex);
			}
			ray.origin = hitVertex.pos + SURFACE_DISTANCE_OFFSET * ray.direction;
			ray.update();

			if (rayHandlingValue <= obj->diffuseThreshold) {
				Vector3f direction = toLight ? ray.direction : prevDirection;
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

GraphicsEngine::LightSourcePoint GraphicsEngine::getRandomLightSourcePoint() {
	size_t lightIndex = rng.rand() * float(lightSources.size());
	GraphicsObject* lightSource = lightSources[lightIndex];

	float sqrtr1 = sqrt(rng.rand());
	float r2 = rng.rand();
	Vector3f barycentricCoords({1.0f - sqrtr1, sqrtr1 * (1.0f - r2), sqrtr1 * r2});

	size_t ti = rng.rand() * float(lightSource->triangles.size());
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
