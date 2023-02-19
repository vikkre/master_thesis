#include "graphics_engine.h"

#define SURFACE_DISTANCE_OFFSET 0.01f


void threadRender(GraphicsEngine* graphicsEngine, const Matrix4f& viewInverse, const Matrix4f& projInverse, const Vector3f& origin) {
	graphicsEngine->render(viewInverse, projInverse, origin);
}


GraphicsEngine::GraphicsEngine()
:imageSize(), image(), camera(nullptr), objects(), scene(), rng() {}

GraphicsEngine::~GraphicsEngine() {}

void GraphicsEngine::parseInput(const InputEntry& inputEntry) {
	raysPerPixel = inputEntry.get<unsigned int>("raysPerPixel");
	visionJumpCount = inputEntry.get<unsigned int>("visionJumpCount");
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

	Vector3f color;
	for (unsigned int i = 0; i < raysPerPixel; ++i) {
		color += traceRay(Ray(origin, direction));
	}
	color /= float(raysPerPixel);
	color *= 2.0f * M_PI;
	for (unsigned int i = 0; i < 3; ++i)
		color[i] = std::clamp(color[i], 0.0f, 1.0f);

	unsigned int index = (x + y * imageSize[0]) * 3;
	for (unsigned int i = 0; i < 3; ++i)
		image[index + i] = (char) (color[i] * 255.0f);
}

Vector3f GraphicsEngine::traceRay(Ray ray) {
	Mesh::Vertex hitVertex;
	const GraphicsObject* obj;

	Vector3f color({1.0f, 1.0f, 1.0f});
	bool lightHit = false;
	bool backfaceCulling = true;

	for (unsigned int i = 0; i < visionJumpCount; ++i) {
		if (scene.traceRay(ray, hitVertex, obj)) {
			float ndotd = hitVertex.normal.dot(ray.direction);
			if (backfaceCulling && ndotd > 0.0f) {
				ray.origin = hitVertex.pos + SURFACE_DISTANCE_OFFSET * ray.direction;
				continue;
			}
			backfaceCulling = false;

			if (obj->lightSource) {
				if (hitVertex.normal.dot(ray.direction) > 0.0f) break;
				color *= obj->color * obj->lightStrength;
				lightHit = true;
				break;
			} else {
				color *= obj->color;
				float rayHandlingValue = rng.rand();

				if (rayHandlingValue <= obj->diffuseThreshold) {
					ray.direction = rng.randomNormalDirection(hitVertex.normal);
				} else if (rayHandlingValue <= obj->reflectThreshold) {
					ray.direction = reflect(ray.direction, hitVertex.normal);
				} else if (rayHandlingValue <= obj->transparentThreshold) {
					ray.direction = customRefract(ray.direction, hitVertex.normal, obj->refractionIndex);
				}

				ray.origin = hitVertex.pos + SURFACE_DISTANCE_OFFSET * ray.direction;
				ray.update();
			}

		} else {
			break;
		}
	}

	if (lightHit) return color;
	else return Vector3f({0.0f, 0.0f, 0.0f});
}
