#extension GL_EXT_scalar_block_layout: enable


layout(binding = 0, set = 0, rgba8) uniform image2D rawImage;
layout(binding = 1, set = 0, rgba8) uniform image2D resultImage;
layout(binding = 2, set = 0, scalar) uniform Settings {
	float historicalBlendingFactor;
	float smoothstepEdge0;
	float smoothstepEdge1;
} settings;
layout(binding = 3, set = 0, rgba8) uniform image2D temporalImage;
layout(binding = 4, set = 0, rgba8) uniform image2D historicalBuffer;
layout(binding = 5, set = 0, rgba8) uniform image2D fireflyFreeImage;
layout(binding = 6, set = 0, rgba8) uniform image2D confidenceBuffer;
layout(binding = 7, set = 0, rgba8) uniform image2D kernelBuffer;


float smoothstep(float x) {
	if (x < settings.smoothstepEdge0) return 0.0;
	if (x >= settings.smoothstepEdge1) return 1.0;

	x = (x - settings.smoothstepEdge0) / (settings.smoothstepEdge1 - settings.smoothstepEdge0);
	return x * x * (3.0 - 2.0 * x);
}

vec3 smoothstep(vec3 x) {
	return vec3(
		smoothstep(x.x),
		smoothstep(x.y),
		smoothstep(x.z)
	);
}
