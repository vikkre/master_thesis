#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require


struct ObjectProperties {
	mat4 model;
	vec3 color;
	float reflect;
	uint64_t vertexAddress;
	uint64_t indexAddress;
};

struct Vertex {
  vec3 pos;
  vec3 normal;
};

layout(binding = 1, set = 0, scalar) buffer ObjectProperties_ { ObjectProperties l[]; } objProps;
layout(buffer_reference, scalar) buffer Vertices { Vertex v[]; };
layout(buffer_reference, scalar) buffer Indices { ivec3 i[]; };

struct RayPayload {
	bool miss;
	vec3 pos;
	vec3 normal;
	vec3 color;
};
layout(location = 0) rayPayloadInEXT RayPayload rayPayload;
hitAttributeEXT vec2 attribs;

void main() {
	ObjectProperties obj = objProps.l[gl_InstanceID];
	Indices indices      = Indices(obj.indexAddress);
	Vertices vertices    = Vertices(obj.vertexAddress);

	const vec3 barycentricCoords = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);

	ivec3 ind = indices.i[gl_PrimitiveID];
	Vertex v0 = vertices.v[ind.x];
	Vertex v1 = vertices.v[ind.y];
	Vertex v2 = vertices.v[ind.z];

	vec3 pos = v0.pos * barycentricCoords.x + v1.pos * barycentricCoords.y + v2.pos * barycentricCoords.z;
	pos = (obj.model * vec4(pos, 1.0)).xyz;

	rayPayload.miss = false;
	rayPayload.pos = pos;
	rayPayload.normal = v0.normal * barycentricCoords.x + v1.normal * barycentricCoords.y + v2.normal * barycentricCoords.z;
	rayPayload.color = obj.color;
}
