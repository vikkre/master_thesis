#extension GL_EXT_ray_tracing: enable
#extension GL_EXT_nonuniform_qualifier: enable
#extension GL_EXT_scalar_block_layout: enable
#extension GL_EXT_buffer_reference2: require
#extension GL_EXT_shader_explicit_arithmetic_types_int64: require


#include "v1.glsl"


layout(set = 0, binding = 0, scalar) uniform RenderSettings {
	uint visionJumpCount;
	uint visionRayPerPixelCount;
} renderSettings;
layout(set = 0, binding = 1, r32ui) uniform uimage2D rawImageRed;
layout(set = 0, binding = 2, r32ui) uniform uimage2D rawImageGreen;
layout(set = 0, binding = 3, r32ui) uniform uimage2D rawImageBlue;
layout(set = 0, binding = 4, rgba8) uniform image2D finalImage;
