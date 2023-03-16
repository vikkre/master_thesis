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
	vec3 direction;
	vec3 color;
	uint hit;
	uint lightSource;
};


struct Surfel {
	vec3 rayDirection;
	vec3 hitRadiance;
	float hitDistance;
	bool hit;
};


layout(set = 0, binding = 0, scalar) uniform RenderSettings {
	uint visionJumpCount;
	uint lightJumpCount;
	uint maxDepth;

	float hysteresis;
	uvec3 probeCount;
	uint totalProbeCount;
	vec3 probeStartCorner;
	vec3 betweenProbeDistance;
	uint perProbeRayCount;
	float maxProbeRayDistance;
	uint probeSampleSideLength;
	float depthSharpness;
	float normalBias;
	uint linearBlending;
	float energyPreservation;

	uint candidateCount;
	uint sampleCount;

	uint probeCandidateCount;
	uint probeSampleCount;
	uint probeVisionJumpCount;
	uint probeLightJumpCount;
	uint probeMaxDepth;
} renderSettings;
layout(set = 0, binding = 1, scalar) buffer SB { Surfel s[]; } surfels;
layout(set = 0, binding = 2, rgba8) uniform image2D irradianceBuffer;
layout(set = 0, binding = 3, rg16f) uniform image2D depthBuffer;
layout(set = 0, binding = 4) uniform sampler2D irradianceSampler;
layout(set = 0, binding = 5) uniform sampler2D depthSampler;
layout(set = 0, binding = 6, rgba8) uniform image2D finalImage;
layout(set = 0, binding = 7, scalar) buffer BitterliRayPayloads_ { BitterliRayPayload r[]; } rayPayloads;
layout(set = 0, binding = 8, scalar) buffer SpatialReservoirs_ { Reservoir r[]; } spatialReservoirs;
layout(set = 0, binding = 9, scalar) buffer BitterliRayPayloadsProbe_ { BitterliRayPayload r[]; } probeRayPayloads;
layout(set = 0, binding = 10, scalar) buffer SpatialReservoirsProbe_ { Reservoir r[]; } probeSpatialReservoirs;

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

uint getProbeReservoirIndex(uvec2 launchID, uvec2 launchSize, uint sampleIndex) {
	uint y_size = renderSettings.probeSampleCount;
	uint x_size = launchSize.y * y_size;
	return launchID.x * x_size + launchID.y * y_size + sampleIndex;
}

struct HitPoint {
	vec3 pos;
	vec3 normal;
	vec3 cumulativeColor;
	bool diffuse;
	bool lightHit;
};

#ifndef COMPUTE_SHADER

#define MAX_PATH_LENGTH 20
#define VISION_HP_INDEX 0
#define LIGHT_HP_INDEX 1

HitPoint hitPoints[2][MAX_PATH_LENGTH];


uint tracePath(inout RNG rng, RaySendInfo rayInfo, uint startDepth, uint maxDepth, bool isLightRay) {
	uint hp_index = isLightRay ? LIGHT_HP_INDEX : VISION_HP_INDEX;
	vec3 color = vec3(1.0);
	uint pathDepth = startDepth;

	if (startDepth > 0) {
		color = hitPoints[hp_index][startDepth - 1].cumulativeColor;
	}

	for (uint i = startDepth; i < maxDepth; ++i) {
		traceRay(rayInfo);

		if (!rayPayload.hit) {
			break;
		}

		pathDepth = i + 1;
		uint traceValue = handleHit(rayInfo, rng);

		if (traceValue == DIFFUSE_VALUE) {
			vec3 direction = isLightRay ? rayInfo.prevDirection : rayInfo.direction;
			color *= rayPayload.color * dot(direction, rayPayload.normal);
			hitPoints[hp_index][i].diffuse = true;
		} else {
			hitPoints[hp_index][i].diffuse = false;
		}

		hitPoints[hp_index][i].pos = rayPayload.pos;
		hitPoints[hp_index][i].normal = rayPayload.normal;
		hitPoints[hp_index][i].cumulativeColor = color;
		hitPoints[hp_index][i].lightHit = false;

		if (rayPayload.lightSource) {
			if (dot(rayInfo.prevDirection, rayPayload.normal) <= 0.0) {
				hitPoints[hp_index][i].lightHit = true;
				break;
			}
		}
	}

	return pathDepth;
}

#endif
