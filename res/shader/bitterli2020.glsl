#extension GL_EXT_ray_tracing: enable
#extension GL_EXT_nonuniform_qualifier: enable
#extension GL_EXT_scalar_block_layout: enable
#extension GL_EXT_buffer_reference2: require
#extension GL_EXT_shader_explicit_arithmetic_types_int64: require


#include "v1.glsl"


#define DIRECTION_COUNT 9
const ivec2 DIRECTIONS[DIRECTION_COUNT] = {
	ivec2(-1, -1),
	ivec2(-1,  0),
	ivec2(-1,  1),
	ivec2( 0, -1),
	ivec2( 0,  0),
	ivec2( 0,  1),
	ivec2( 1, -1),
	ivec2( 1,  0),
	ivec2( 1,  1)
};


struct Sample {
	LightSourcePoint lsp;
	float weight;
};

struct Reservoir {
	Sample y;
	float w_sum;
	uint M;
	float W;
};

struct BitterliRayPayload {
	vec3 pos;
	vec3 normal;
	vec3 color;
	bool hit;
	bool lightSource;
};

layout(set = 0, binding = 0, scalar) uniform RenderSettings {
	uint visionJumpCount;
	uint candidateCount;
	uint sampleCount;
} renderSettings;
layout(set = 0, binding = 1, rgba8) uniform image2D finalImage;
layout(set = 0, binding = 2, scalar) buffer BitterliRayPayloads_ { BitterliRayPayload r[]; } rayPayloads;
layout(set = 0, binding = 3, scalar) buffer spatialReservoirs_ { Reservoir r[]; } spatialReservoirs;
layout(set = 0, binding = 4, scalar) buffer prevTemporalReservoirs_ { Reservoir r[]; } prevTemporalReservoirs;
layout(set = 0, binding = 5, scalar) buffer nextTemporalReservoirs_ { Reservoir r[]; } nextTemporalReservoirs;

Reservoir createReservoir() {
	Reservoir r;
	r.w_sum = 0.0;
	r.M = 0;
	r.W = 0.0;
	return r;
}

void updateReservoir(inout Reservoir r, inout RNG rng, Sample x_i, float w_i) {
	r.w_sum += w_i;
	r.M += 1;
	if (rand(rng) < (w_i / r.w_sum)) r.y = x_i;
}

uint getPayloadIndex(uvec2 launchID, uvec2 launchSize) {
	return launchID.x + launchID.y * launchSize.x;
}

uint getReservoirIndex(uvec2 launchID, uvec2 launchSize, uint i) {
	return launchID.x + launchID.y * launchSize.x + i * launchSize.x * launchSize.y;
}
