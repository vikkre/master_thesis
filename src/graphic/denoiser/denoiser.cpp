#include "denoiser.h"


Denoiser::Denoiser(Device* device)
:device(device), inputImages(device), outputImages(nullptr) {}

Denoiser::~Denoiser() {}

void Denoiser::init() {
	createInputImages();
	initDenoiser();
}

void Denoiser::initDenoiser() {}

MultiBufferDescriptor<ImageBuffer>* Denoiser::getInputImageBuffer() {
	return &inputImages;
}

void Denoiser::setOutputImageBuffer(MultiBufferDescriptor<ImageBuffer>* outputImageBuffer) {
	outputImages = outputImageBuffer;
}

void Denoiser::createInputImages() {
	inputImages.bufferProperties = outputImages->bufferProperties;
	inputImages.init();
}
