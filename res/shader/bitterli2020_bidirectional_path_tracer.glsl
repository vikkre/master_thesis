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
	uint hit;
	uint lightSource;
};

struct HitPoint {
	vec3 pos;
	vec3 normal;
	vec3 cumulativeColor;
	bool diffuse;
	bool lightHit;
};


layout(set = 0, binding = 0, scalar) uniform RenderSettings {
	uint visionJumpCount;
	uint lightJumpCount;
	uint maxDepth;
	uint candidateCount;
	uint sampleCount;
} renderSettings;
layout(set = 0, binding = 1, rgba8) uniform image2D finalImage;
layout(set = 0, binding = 2, scalar) buffer BitterliRayPayloads_ { BitterliRayPayload r[]; } rayPayloads;
layout(set = 0, binding = 3, scalar) buffer SpatialReservoirs_ { Reservoir r[]; } spatialReservoirs;

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
	return launchID.x * launchSize.y + launchID.y;
}

uint getReservoirIndex(uvec2 launchID, uvec2 launchSize, uint sampleIndex) {
	uint y_size = renderSettings.sampleCount;
	uint x_size = launchSize.y * y_size;
	return launchID.x * x_size + launchID.y * y_size + sampleIndex;
}

bool isOccluded(vec3 startPos, vec3 endPos) {
	vec3 stretch = endPos - startPos;
	float dist = length(stretch);
	vec3 direction = stretch / dist;

	uint rayFlags = gl_RayFlagsOpaqueEXT;
	uint cullMask = 0xFF;
	float tmin = 0.001;
	shadowed = true;

	traceRayEXT(topLevelAS, rayFlags, cullMask, 1, 0, 1, startPos, tmin, direction, dist, 1);

	return shadowed;
}
