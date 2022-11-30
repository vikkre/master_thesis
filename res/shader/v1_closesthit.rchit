#version 460

#define HIT_OR_MISS_SHADER
#include "v1.glsl"


hitAttributeEXT vec2 attribs;

void main() {
	ObjectProperties obj = objProps.l[gl_InstanceID];
	Indices indices      = Indices(obj.indexAddress);
	Vertices vertices    = Vertices(obj.vertexAddress);

	vec3 barycentricCoords = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);

	ivec3 ind = indices.i[gl_PrimitiveID];
	Vertex v0 = vertices.v[ind.x];
	Vertex v1 = vertices.v[ind.y];
	Vertex v2 = vertices.v[ind.z];

	vec3 pos = v0.pos * barycentricCoords.x + v1.pos * barycentricCoords.y + v2.pos * barycentricCoords.z;
	pos = (obj.model * vec4(pos, 1.0)).xyz;

	vec3 normal = v0.normal * barycentricCoords.x + v1.normal * barycentricCoords.y + v2.normal * barycentricCoords.z;
	normal = normalize((obj.model * vec4(normal, 0.0)).xyz);

	rayPayload.hit = true;
	rayPayload.pos = pos;
	rayPayload.normal = normal;

	rayPayload.diffuseThreshold = obj.diffuseThreshold;
	rayPayload.reflectThreshold = obj.reflectThreshold;
	rayPayload.transparentThreshold = obj.transparentThreshold;
	rayPayload.refractionIndex = obj.refractionIndex;
	if (obj.lightSource == 1) {
		rayPayload.lightSource = true;
		rayPayload.color = obj.color * obj.lightStrength;
	} else {
		rayPayload.lightSource = false;
		rayPayload.color = obj.color;
	}
}
