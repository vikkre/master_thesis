#include "graphics_engine.h"


void threadRender(GraphicsEngine* graphicsEngine, unsigned int t, unsigned int startY, unsigned int endY, const Matrix4f& viewInverse, const Matrix4f& projInverse, const Vector3f& origin) {
	graphicsEngine->render(t, startY, endY, viewInverse, projInverse, origin);
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

	std::vector<unsigned int> ys(threadCount + 1);
	for (unsigned int t = 0; t <= threadCount; ++t) {
		ys[t] = imageSize[1] * float(t) / float(threadCount);
	}

	std::vector<std::thread> threads;
	threads.reserve(threadCount);

	for (unsigned int t = 0; t < threadCount; ++t) {
		threads.push_back(std::thread(threadRender, this, t, ys[t], ys[t + 1], viewInverse, projInverse, origin));
	}

	for (std::thread& th: threads) th.join();
}

void GraphicsEngine::render(unsigned int t, unsigned int startY, unsigned int endY, const Matrix4f& viewInverse, const Matrix4f& projInverse, const Vector3f& origin) {
	for (unsigned int x = 0; x < imageSize[0]; ++x) {
		if (x % 100 == 0 && x != 0) std::cout << "Thread " << t << ": " << float(x) / float(imageSize[0]) << " done" << std::endl;
		for (unsigned int y = startY; y < endY; ++y) {
			renderPixel(x, y, viewInverse, projInverse, origin);
		}
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
		color += renderRay(origin, direction);
	}
	color /= float(raysPerPixel);
	color *= 5.0f;

	unsigned int index = (x + y * imageSize[0]) * 3;
	for (unsigned int i = 0; i < 3; ++i)
		image[index + i] = (char) (color[i] * 255.0f);
}

Vector3f GraphicsEngine::renderRay(Vector3f origin, Vector3f direction) {
	Mesh::Vertex hitVertex;
	const GraphicsObject* obj;

	Vector3f color({1.0f, 1.0f, 1.0f});
	bool lightHit = false;
	bool backfaceCulling = true;

	for (unsigned int i = 0; i < visionJumpCount; ++i) {
		if (scene.traceRay(origin, direction, hitVertex, obj)) {
			float ndotd = hitVertex.normal.dot(direction);
			if (backfaceCulling && ndotd > 0.0f) {
				origin = hitVertex.pos + 0.01f * direction;
				continue;
			}
			backfaceCulling = false;

			if (obj->lightSource) {
				color *= obj->color * obj->lightStrength;
				lightHit = true;
				break;
			} else {
				color *= obj->color;
				float rayHandlingValue = rng.rand();

				origin = hitVertex.pos + 0.1f * hitVertex.normal;

				if (rayHandlingValue <= obj->diffuseThreshold) {
					direction = rng.randomNormalDirection(hitVertex.normal);
				} else if (rayHandlingValue <= obj->reflectThreshold) {
					direction = reflect(direction, hitVertex.normal);
				} else if (rayHandlingValue <= obj->transparentThreshold) {
					direction = customRefract(direction, hitVertex.normal, obj->refractionIndex);
				}
			}

		} else {
			break;
		}
	}

	if (lightHit) return color;
	else return Vector3f({0.0f, 0.0f, 0.0f});
}
