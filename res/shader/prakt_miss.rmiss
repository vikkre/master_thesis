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
} globalData;

struct RayPayload {
	vec3 color;
	float distance;
	vec3 normal;
	float reflector;
};
layout(location = 0) rayPayloadInEXT RayPayload rayPayload;

void main() {
	rayPayload.color = globalData.backgroundColor;
	rayPayload.distance = -1.0;
	rayPayload.normal = vec3(0.0);
	rayPayload.reflector = 0.0;
}
