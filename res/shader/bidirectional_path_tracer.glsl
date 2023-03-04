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
	vec3 cumulativeColor;
	bool diffuse;
	bool lightHit;
};
