#extension GL_EXT_ray_tracing: enable
#extension GL_EXT_nonuniform_qualifier: enable
#extension GL_EXT_scalar_block_layout: enable
#extension GL_EXT_buffer_reference2: require
#extension GL_EXT_shader_explicit_arithmetic_types_int64: require

#include "helper.glsl"


#define LIGHT_POSITION vec3(0, 4.8, 0)


struct ObjectProperties {
	mat4 model;
	vec3 color;
	uint64_t vertexAddress;
	uint64_t indexAddress;
	float diffuseThreshold;
	float reflectThreshold;
	float transparentThreshold;
	float refractionIndex;
};

struct Vertex {
	vec3 pos;
	vec3 normal;
};

layout(set = 1, binding = 0) uniform accelerationStructureEXT topLevelAS;
layout(set = 1, binding = 1, scalar) buffer ObjectProperties_ { ObjectProperties l[]; } objProps;
layout(set = 1, binding = 2, scalar) uniform RayTraceingSettings {
	mat4 viewInverse;
	mat4 projInverse;
	mat4 view;
	mat4 proj;
} rtSettings;

layout(buffer_reference, scalar) buffer Vertices { Vertex v[]; };
layout(buffer_reference, scalar) buffer Indices { ivec3 i[]; };

#ifndef COMPUTE_SHADER
struct RayPayload {
	bool hit;
	vec3 pos;
	vec3 normal;
	vec3 color;
	float diffuseThreshold;
	float reflectThreshold;
	float transparentThreshold;
	float refractionIndex;
};

#ifdef HIT_OR_MISS_SHADER
layout(location = 0) rayPayloadInEXT RayPayload rayPayload;
#else
layout(location = 0) rayPayloadEXT RayPayload rayPayload;
#endif

struct RaySendInfo {
	vec3 origin;
	vec3 direction;
	bool backfaceCulling;
};

RaySendInfo getLightRayRandom(vec3 seed, int i) {
	RaySendInfo rayInfo;
	rayInfo.origin = LIGHT_POSITION;
	rayInfo.direction = randomNormal(seed, i, 1);
	rayInfo.backfaceCulling = false;
	return rayInfo;
}

RaySendInfo getLightRayUniform(uint index, uint maxRayCount) {
	RaySendInfo rayInfo;
	rayInfo.origin = LIGHT_POSITION;
	rayInfo.direction = sphericalFibonacci(float(index), float(maxRayCount));
	rayInfo.backfaceCulling = false;
	return rayInfo;
}

RaySendInfo getVisionRay(uvec2 launchPoint, uvec2 launchSize) {
	vec2 pixelCenter = vec2(launchPoint.xy) + vec2(0.5);
	vec2 inUV = pixelCenter/vec2(launchSize.xy);
	vec2 d = inUV * 2.0 - 1.0;

	RaySendInfo rayInfo;
	rayInfo.origin = (rtSettings.viewInverse * vec4(vec3(0), 1)).xyz;
	vec3 target = (rtSettings.projInverse * vec4(d.x, d.y, 1, 1)).xyz;
	rayInfo.direction = normalize((rtSettings.viewInverse * vec4(normalize(target), 0)).xyz);
	rayInfo.backfaceCulling = true;
	return rayInfo;
}

#define DIFFUSE_VALUE 0
#define REFLECT_VALUE 1
#define TRANSPARENT_VALUE 2

void traceRay(RaySendInfo rayInfo) {
	uint rayFlags = gl_RayFlagsOpaqueEXT;
	uint cullMask = 0xFF;
	float tmin = 0.001;
	float tmax = 10000.0;

	vec3 origin = rayInfo.origin;

	for (int i = 0; i < 4; i++) {
		traceRayEXT(topLevelAS, rayFlags, cullMask, 0, 0, 0, origin, tmin, rayInfo.direction, tmax, 0);

		bool culling = rayPayload.hit && rayInfo.backfaceCulling && dot(rayPayload.normal, rayInfo.direction) > 0;
		if (!culling) break;
		else origin = rayPayload.pos;
	}
}

uint handleHit(inout RaySendInfo rayInfo, int i) {
	float rayHandlingValue = rand(rayInfo.origin, i, 0);

	rayInfo.origin = rayPayload.pos;
	rayInfo.backfaceCulling = false;

	if (rayHandlingValue <= rayPayload.diffuseThreshold) {
		rayInfo.direction = randomNormalDirection(rayInfo.origin, i, 1, rayPayload.normal);
		return DIFFUSE_VALUE;

	} else if (rayHandlingValue <= rayPayload.reflectThreshold) {
		rayInfo.direction = reflect(rayInfo.direction, rayPayload.normal);
		return REFLECT_VALUE;

	} else if (rayHandlingValue <= rayPayload.transparentThreshold) {
		rayInfo.direction = customRefract(rayInfo.direction, rayPayload.normal, rayPayload.refractionIndex);
		return TRANSPARENT_VALUE;

	}
}

float getIlluminationByShadowtrace(vec3 pos, vec3 normal) {
	vec3 toLight = LIGHT_POSITION - pos;
	vec3 direction = normalize(toLight);
	float lightStrength = dot(direction, normal);
	if (lightStrength <= 0.0) return 0.0;
	float distToLight = length(toLight);

	RayPayload tmp = rayPayload;

	uint rayFlags = gl_RayFlagsOpaqueEXT;
	uint cullMask = 0xFF;
	float tmin = 0.001;
	traceRayEXT(topLevelAS, rayFlags, cullMask, 0, 0, 0, pos, tmin, direction, distToLight, 0);

	bool hit = rayPayload.hit;
	rayPayload = tmp;

	if (hit) return 0.0;

	return lightStrength;
}

#endif
