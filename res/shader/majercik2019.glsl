#extension GL_EXT_ray_tracing: enable
#extension GL_EXT_nonuniform_qualifier: enable
#extension GL_EXT_scalar_block_layout: enable
#extension GL_EXT_buffer_reference2: require
#extension GL_EXT_shader_explicit_arithmetic_types_int64: require


#include "v1.glsl"


struct Surfel {
	vec3 rayDirection;
	vec3 hitRadiance;
	float hitDistance;
	bool hit;
};


layout(set = 0, binding = 0, scalar) uniform RenderSettings {
	uint lightJumpCount;
	uint visionJumpCount;
	float betweenProbeDistance;
	uint singleDirectionProbeCount;
	uint totalProbeCount;
	uint perProbeRayCount;
	float maxProbeRayDistance;
	uint probeSampleSideLength;
	float depthSharpness;
	float normalBias;
	float crushThreshold;
	uint linearBlending;
	float energyPreservation;
	float texelGetProbeDirectionFactor;
	float texelGetNormalFactor;
	uint shadowCountProbe;
	uint shadowCountVision;
} renderSettings;
layout(set = 0, binding = 1, scalar) buffer SB { Surfel s[]; } surfels;
layout(set = 0, binding = 2, rgba8) uniform image2D irradianceBuffer;
layout(set = 0, binding = 3, rgba8) uniform image2D depthBuffer;
layout(set = 0, binding = 4) uniform sampler2D irradianceSampler;
layout(set = 0, binding = 5) uniform sampler2D depthSampler;
layout(set = 0, binding = 6, rgba8) uniform image2D finalImage;
