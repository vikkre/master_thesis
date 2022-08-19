#include "graphics_engine.h"


GraphicsEngine::GraphicsEngine()
:device(), swapchain(&device), renderer(&device),
commandBuffersRecorded(false) {
	device.renderInfo.backgroundColor = Vector3f({0.0f, 0.0f, 0.0f});
	device.renderInfo.camera.position = Vector3f({0.0f, 0.0f, 0.0f});
	device.renderInfo.camera.lookAt   = Vector3f({0.0f, 0.0f, 0.0f});
}

GraphicsEngine::~GraphicsEngine() {}

void GraphicsEngine::init() {
	SDLWindow* sdlWindow = new SDLWindow();
	sdlWindow->windowName = "Ray Tracing Test";
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
	// rtpipeline.initTlas();

	// rtpipeline.init();
	// device.renderInfo.renderPipeline = &rtpipeline;

	renderer.init();

	swapchain.recordCommandBuffers([this](size_t index, VkCommandBuffer commandBuffer) {
		this->renderer.cmdRender(index, commandBuffer);
	});
}

void GraphicsEngine::render() {
	// if (!commandBuffersRecorded) {
	// 	swapchain.recordCommandBuffers([this](size_t index, VkCommandBuffer commandBuffer) {
	// 		this->renderer.cmdRender(index, commandBuffer);
	// 	});
	// 	commandBuffersRecorded = true;
	// }

	float aspect = device.renderInfo.swapchainExtend.width / (float) device.renderInfo.swapchainExtend.height;
	// pipeline.globalData.projectionMatrix = device.renderInfo.camera.getProjectionMatrix(aspect);
	// pipeline.globalData.viewMatrix = device.renderInfo.camera.getViewMatrix();

	Matrix4f projectionMatrix = device.renderInfo.camera.getProjectionMatrix(aspect);
	Matrix4f viewMatrix = device.renderInfo.camera.getViewMatrix();

	renderer.globalData.projInverse = projectionMatrix.inverseMatrix();
	renderer.globalData.viewInverse = viewMatrix.inverseMatrix();
	renderer.globalData.proj = projectionMatrix;
	renderer.globalData.view = viewMatrix;

	swapchain.render([this](size_t index) {
		this->renderer.updateUniforms(index);
	});
}
