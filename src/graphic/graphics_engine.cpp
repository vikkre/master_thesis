#include "graphics_engine.h"


GraphicsEngine::GraphicsEngine()
:device(), swapchain(&device), rtpipeline(&device),
commandBuffersRecorded(false) {}

GraphicsEngine::~GraphicsEngine() {}

void GraphicsEngine::init() {
	SDLWindow* sdlWindow = new SDLWindow();
	sdlWindow->windowName = "Computer Graphik Praktikum";
	sdlWindow->windowSize = Vector2i({1600, 900});
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

	// rtpipeline.objects = pipeline.objects;

	// pipeline.init();
	// device.renderInfo.renderPipeline = &pipeline;

	// rtpipeline.init();
	// device.renderInfo.renderPipeline = &rtpipeline;
}

void GraphicsEngine::initTlas() {
	rtpipeline.initTlas();

	rtpipeline.init();
	device.renderInfo.renderPipeline = &rtpipeline;
}

void GraphicsEngine::render() {
	if (!commandBuffersRecorded) {
		swapchain.recordCommandBuffers();
		commandBuffersRecorded = true;
	}

	float aspect = device.renderInfo.swapchainExtend.width / (float) device.renderInfo.swapchainExtend.height;
	// pipeline.globalData.projectionMatrix = device.renderInfo.camera.getProjectionMatrix(aspect);
	// pipeline.globalData.viewMatrix = device.renderInfo.camera.getViewMatrix();

	Matrix4f projectionMatrix = device.renderInfo.camera.getProjectionMatrix(aspect);
	Matrix4f viewMatrix = device.renderInfo.camera.getViewMatrix();

	rtpipeline.globalData.projInverse = projectionMatrix.inverseMatrix();
	rtpipeline.globalData.viewInverse = viewMatrix.inverseMatrix();
	rtpipeline.globalData.proj = projectionMatrix;
	rtpipeline.globalData.view = viewMatrix;

	swapchain.render();
}
