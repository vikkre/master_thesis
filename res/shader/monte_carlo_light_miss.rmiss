#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_scalar_block_layout : enable

struct RayPayload {
	bool miss;
	vec3 pos;
	vec3 color;
};
layout(location = 0) rayPayloadInEXT RayPayload rayPayload;

void main() {
	rayPayload.miss = true;
}
