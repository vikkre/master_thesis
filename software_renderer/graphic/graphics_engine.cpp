#include "graphics_engine.h"

#define SURFACE_DISTANCE_OFFSET 0.01f


void threadRender(GraphicsEngine* graphicsEngine, const Matrix4f& viewInverse, const Matrix4f& projInverse, const Vector3f& origin) {
	graphicsEngine->render(viewInverse, projInverse, origin);
}


GraphicsEngine::GraphicsEngine()
:imageSize(), image(), camera(nullptr),
objects(), lightSources(), scene(),
threadCount(1), pixelCounter(0), renderer(nullptr) {}

GraphicsEngine::~GraphicsEngine() {
	if (renderer != nullptr) delete renderer;
}

void GraphicsEngine::init(Renderer* renderer, unsigned int threadCount) {
	this->renderer = renderer;
	this->threadCount = threadCount;

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

	Renderer::PixelRenderData prd;
	prd.scene = &scene;
	prd.objects = &objects;
	prd.lightSources = &lightSources;

	prd.imageSize = imageSize;
	prd.origin = origin;
	prd.viewInverse = viewInverse;
	prd.projInverse = projInverse;

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

		prd.pixel[0] = currentPixel % imageSize[0];
		prd.pixel[1] = currentPixel / imageSize[0];
		Vector3f finalColor = renderer->renderPixel(prd);

		for (unsigned int i = 0; i < 3; ++i)
			finalColor[i] = std::clamp(finalColor[i], 0.0f, 1.0f);

		unsigned int index = (prd.pixel[0] + prd.pixel[1] * imageSize[0]) * 3;
		for (unsigned int i = 0; i < 3; ++i)
			image[index + i] = (char) (finalColor[i] * 255.0f);
	}
}
