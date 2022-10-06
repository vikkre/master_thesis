#extension GL_EXT_ray_tracing: enable
#extension GL_EXT_nonuniform_qualifier: enable
#extension GL_EXT_scalar_block_layout: enable
#extension GL_EXT_buffer_reference2: require
#extension GL_EXT_shader_explicit_arithmetic_types_int64: require


#include "helper.glsl"
#include "v1.glsl"


#define MAX_LIGHT_JUMPS 100


layout(set = 0, binding = 0, scalar) uniform RenderSettings {
	uint resultType;
	float scaling;
	uint lightJump;
} renderSettings;
layout(set = 0, binding = 1, rgba8) uniform image2D image;
