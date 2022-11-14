#extension GL_EXT_ray_tracing: enable
#extension GL_EXT_nonuniform_qualifier: enable
#extension GL_EXT_scalar_block_layout: enable
#extension GL_EXT_buffer_reference2: require
#extension GL_EXT_shader_explicit_arithmetic_types_int64: require


#include "v1.glsl"


layout(set = 0, binding = 0, scalar) uniform RenderSettings {
	uint visionJumpCount;
	uint shadowTraceCount;
	uint candidateCount;
	uint usePrimitiveShadowTrace;
} renderSettings;
layout(set = 0, binding = 1, rgba8) uniform image2D finalImage;

struct Sample {
	LightSourcePoint lsp;
	float weight;
};

struct Reservoir {
	Sample y;
	float w_sum;
	uint M;
};

Reservoir createReservoir() {
	Reservoir r;
	r.w_sum = 0.0;
	r.M = 0;
	return r;
}

void updateReservoir(inout Reservoir r, inout RNG rng, Sample x_i, float w_i) {
	r.w_sum += w_i;
	r.M += 1;
	if (rand(rng) < (w_i / r.w_sum)) r.y = x_i;
}
