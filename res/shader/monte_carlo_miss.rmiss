#version 460

#include "monte_carlo.glsl"


void main() {
	rayPayload.miss = true;
	rayPayload.color = renderSettings.backgroundColor;
}
