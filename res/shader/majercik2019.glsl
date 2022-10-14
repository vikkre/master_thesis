#extension GL_EXT_ray_tracing: enable
#extension GL_EXT_nonuniform_qualifier: enable
#extension GL_EXT_scalar_block_layout: enable
#extension GL_EXT_buffer_reference2: require
#extension GL_EXT_shader_explicit_arithmetic_types_int64: require


#include "helper.glsl"
#include "v1.glsl"


struct Surfel {
	vec3 rayDirection;
	vec3 hitRadiance;
	float hitDistance;
	bool hit;
};


layout(set = 0, binding = 0, scalar) uniform RenderSettings {
	vec3 lightPosition;
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
} renderSettings;
layout(set = 0, binding = 1, scalar) buffer SB { Surfel s[]; } surfels;
layout(set = 0, binding = 2, rgba8) uniform image2D irradianceBuffer;
layout(set = 0, binding = 3, rgba8) uniform image2D depthBuffer;
layout(set = 0, binding = 4) uniform sampler2D irradianceSampler;
layout(set = 0, binding = 5) uniform sampler2D depthSampler;
layout(set = 0, binding = 6, rgba8) uniform image2D finalImage;


#ifndef COMPUTE_SHADER
float getShade(vec3 pos, vec3 normal) {
	vec3 toLight = renderSettings.lightPosition - pos;
	vec3 normToLight = normalize(toLight);
	float lightStrenth = dot(normToLight, normal);
	if (lightStrenth <= 0.0) return 0.0;
	float distToLight = length(toLight);

	RayPayload tmp = rayPayload;

	uint rayFlags = gl_RayFlagsOpaqueEXT;
	uint cullMask = 0xFF;
	float tmin = 0.001;
	traceRayEXT(topLevelAS, rayFlags, cullMask, 0, 0, 0, pos, tmin, normToLight, distToLight, 0);

	if (rayPayload.hit) return 0.0;

	rayPayload = tmp;

	return dot(normalize(renderSettings.lightPosition - pos), normal);

}
#endif
