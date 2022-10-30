#version 460

#define HIT_OR_MISS_SHADER
#include "v1.glsl"


void main() {
	rayPayload.hit = false;
	rayPayload.color = vec3(0.0);
}
