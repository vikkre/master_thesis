#extension GL_EXT_ray_tracing: enable
#extension GL_EXT_nonuniform_qualifier: enable
#extension GL_EXT_scalar_block_layout: enable
#extension GL_EXT_buffer_reference2: require
#extension GL_EXT_shader_explicit_arithmetic_types_int64: require


#include "v1.glsl"


layout(set = 0, binding = 0, scalar) uniform RenderSettings {
	uint visionJumpCount;
	uint lightJumpCount;
	uint raysPerPixelCount;
} renderSettings;
layout(set = 0, binding = 1, rgba8) uniform image2D image;

struct HitPoint {
	vec3 pos;
	vec3 normal;
	vec3 cumulativeColor;
	bool diffuse;
	bool lightHit;
};

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
