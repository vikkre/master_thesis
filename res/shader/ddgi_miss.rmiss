#version 460

#include "ddgi.glsl"


void main() {
	rayPayload.hit = false;
	rayPayload.color = renderSettings.backgroundColor;
}
