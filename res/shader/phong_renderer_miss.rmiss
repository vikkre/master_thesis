#version 460

#include "phong_renderer.glsl"


void main() {
	rayPayload.hit = false;
	rayPayload.color = vec3(0.0);
}
