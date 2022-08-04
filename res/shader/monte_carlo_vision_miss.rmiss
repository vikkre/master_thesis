#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_scalar_block_layout : enable


layout(binding = 2, set = 0, scalar) uniform GlobalData {
	mat4 viewInverse;
	mat4 projInverse;
	mat4 view;
	mat4 proj;
	vec3 backgroundColor;
	vec3 lightPosition;
	uint lightJumpCount;
	uint visionJumpCount;
} globalData;

struct RayPayload {
	bool miss;
	vec3 pos;
	vec3 normal;
	vec3 color;
};
layout(location = 0) rayPayloadInEXT RayPayload rayPayload;

void main() {
	rayPayload.miss = true;
	rayPayload.color = globalData.backgroundColor;
}
