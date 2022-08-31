#include "graphics_engine.h"


GraphicsEngine::GraphicsEngine(const std::string& basepath)
:device(basepath), swapchain(&device), renderer(nullptr), denoisers(),
commandBuffersRecorded(false) {
	device.renderInfo.backgroundColor = Vector3f({0.0f, 0.0f, 0.0f});
	device.renderInfo.camera.position = Vector3f({0.0f, 0.0f, 0.0f});
	device.renderInfo.camera.lookAt   = Vector3f({0.0f, 0.0f, 0.0f});
}

GraphicsEngine::~GraphicsEngine() {
	for (Denoiser* denoiser: denoisers) delete denoiser;
	delete renderer;
}

void GraphicsEngine::init() {
	SDLWindow* sdlWindow = new SDLWindow();
	sdlWindow->windowName = "Ray Tracing Test";
	sdlWindow->windowSize = windowSize;
	sdlWindow->crateWindowFlags = SDLWindow::CREATE_WINDOW_FLAGS_DEFAULT;

	device.window = sdlWindow;
	device.enableValidationLayers = true;
	device.validationLayers = Device::VALIDATION_LAYERS_DEFAULT;
	// device.validationLayers = Device::VALIDATION_LAYERS_EXTENDED;
	device.severityFlags = Device::VALIDATION_SEVERITY_DEFAULT;
	device.deviceExtensions = Device::DEVICE_EXTENSIONS_DEFAULT;
	device.init();

	FuncLoad::init(device.getDevice());

	swapchain.init();
}

void GraphicsEngine::initTlas() {
	MultiBufferDescriptor<ImageBuffer>* currentImageBuffer = swapchain.getInputImageBuffer();
	for (int i = denoisers.size() - 1; i >= 0; --i) {
		Denoiser* denoiser = denoisers[i];

		denoiser->setOutputImageBuffer(currentImageBuffer);
		denoiser->init();
		currentImageBuffer = denoiser->getInputImageBuffer();
	}
	renderer->setOutputImageBuffer(currentImageBuffer);

	renderer->init();

	swapchain.recordCommandBuffers([this](size_t index, VkCommandBuffer commandBuffer) {
		this->renderer->cmdRender(index, commandBuffer);
		for (Denoiser* denoiser: denoisers) {
			denoiser->cmdRender(index, commandBuffer);
		}
	});
}

void GraphicsEngine::render() {
	float aspect = device.renderInfo.swapchainExtend.width / (float) device.renderInfo.swapchainExtend.height;

	Matrix4f projectionMatrix = device.renderInfo.camera.getProjectionMatrix(aspect);
	Matrix4f viewMatrix = device.renderInfo.camera.getViewMatrix();

	renderer->globalData.projInverse = projectionMatrix.inverseMatrix();
	renderer->globalData.viewInverse = viewMatrix.inverseMatrix();
	renderer->globalData.proj = projectionMatrix;
	renderer->globalData.view = viewMatrix;

	swapchain.render([this](size_t index) {
		this->renderer->updateUniforms(index);
		for (Denoiser* denoiser: denoisers) {
			denoiser->updateUniforms(index);
		}
	});
}

void GraphicsEngine::saveLatestImage(const std::string path) {
	swapchain.saveLatestImage(path);
}
