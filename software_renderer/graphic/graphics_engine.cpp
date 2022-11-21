#include "graphics_engine.h"


GraphicsEngine::GraphicsEngine()
:imageSize(), image(), camera(nullptr), objects(), scene(), rng() {}

GraphicsEngine::~GraphicsEngine() {}

void GraphicsEngine::parseInput(const InputEntry& inputEntry) {
	raysPerPixel = inputEntry.get<unsigned int>("raysPerPixel");
	visionJumpCount = inputEntry.get<unsigned int>("visionJumpCount");
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

	for (unsigned int x = 0; x < imageSize[0]; ++x) {
		std::cout << x * imageSize[1] << "/" << imageSize[0] * imageSize[1] << "(" << 100.0f * float(x * imageSize[1]) / float(imageSize[0] * imageSize[1]) << "%) done" << std::endl;
		for (unsigned int y = 0; y < imageSize[1]; ++y) {
			Vector2f pixelCenter = Vector2f({x + 0.5f, y + 0.5f});
			Vector2f inUV = Vector2f({pixelCenter[0] / (float) imageSize[0], pixelCenter[1] / (float) imageSize[1]});
			Vector2f d = (2.0f * inUV) - Vector2f({1.0f, 1.0f});
			Vector3f target = cutVector(projInverse * Vector4f({d[0], d[1], 1.0f, 1.0f})).normalize();
			Vector3f direction = cutVector(viewInverse * expandVector(target, 0.0f)).normalize();

			Vector3f color;
			for (unsigned int i = 0; i < raysPerPixel; ++i) {
				color += renderPixel(origin, direction);
			}
			color /= float(raysPerPixel);

			unsigned int index = (x + y * imageSize[0]) * 3;
			image[index + 0] = (char) (color[0] * 255.0f);
			image[index + 1] = (char) (color[1] * 255.0f);
			image[index + 2] = (char) (color[2] * 255.0f);
		}
	}
}

Vector3f GraphicsEngine::renderPixel(Vector3f origin, Vector3f direction) {
	Mesh::Vertex hitVertex;
	const GraphicsObject* obj;

	Vector3f color({1.0f, 1.0f, 1.0f});
	bool lightHit = false;
	bool backfaceCulling = true;

	unsigned int i = 0;
	for (; i < visionJumpCount; ++i) {
		if (scene.traceRay(origin, direction, hitVertex, obj)) {
			float ndotd = hitVertex.normal.dot(direction);
			if (backfaceCulling && ndotd > 0.0f) {
				origin = hitVertex.pos + 0.01f * direction;
				continue;
			}
			// return 0.5f * (rng.randomNormalDirection(hitVertex.normal) + Vector3f({1.0f, 1.0f, 1.0f}));
			backfaceCulling = false;

			if (obj->lightSource) {
				color[0] *= obj->color[0] * obj->lightStrength;
				color[1] *= obj->color[1] * obj->lightStrength;
				color[2] *= obj->color[2] * obj->lightStrength;
				lightHit = true;
				break;
			} else {
				color[0] *= obj->color[0];
				color[1] *= obj->color[1];
				color[2] *= obj->color[2];
				float rayHandlingValue = rng.rand();

				origin = hitVertex.pos;

				if (rayHandlingValue <= obj->diffuseThreshold) {
					direction = rng.randomNormalDirection(hitVertex.normal);
				} else if (rayHandlingValue <= obj->reflectThreshold) {
					direction = direction - (2.0f * ndotd * hitVertex.normal);
				} else if (rayHandlingValue <= obj->transparentThreshold) {
					float rIndex = obj->refractionIndex;
					if (ndotd > 0.0f) {
						hitVertex.normal *= -1.0f;
					} else {
						rIndex = 1.0f / rIndex;
					}

					float angle = sin(acos(ndotd)) * rIndex;
					if (-1.0f < angle && angle < 1.0f) {
						float k = 1.0f - rIndex * rIndex * (1.0f - ndotd * ndotd);
						if (k < 0.0f) direction = Vector3f({0.0f, 0.0f, 0.0f});
						else direction = rIndex * direction - (rIndex * ndotd + sqrt(k)) * hitVertex.normal;
					} else {
						direction = direction - (2.0f * ndotd * hitVertex.normal);
					}
				}
			}

		} else {
			break;
		}
	}

	if (lightHit) return color;
	else return Vector3f({0.0f, 0.0f, 0.0f});
}
