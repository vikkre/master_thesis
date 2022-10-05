#extension GL_EXT_ray_tracing: enable
#extension GL_EXT_nonuniform_qualifier: enable
#extension GL_EXT_scalar_block_layout: enable
#extension GL_EXT_buffer_reference2: require
#extension GL_EXT_shader_explicit_arithmetic_types_int64: require


struct ObjectProperties {
	mat4 model;
	vec3 color;
	uint64_t vertexAddress;
	uint64_t indexAddress;
	float diffuseThreshold;
	float reflectThreshold;
	float transparentThreshold;
	float refractionIndex;
};

struct Vertex {
	vec3 pos;
	vec3 normal;
};

layout(set = 1, binding = 0) uniform accelerationStructureEXT topLevelAS;
layout(set = 1, binding = 1, scalar) buffer ObjectProperties_ { ObjectProperties l[]; } objProps;
layout(set = 1, binding = 2, scalar) uniform RayTraceingSettings {
	mat4 viewInverse;
	mat4 projInverse;
	mat4 view;
	mat4 proj;
	vec3 backgroundColor;
} rtSettings;

layout(buffer_reference, scalar) buffer Vertices { Vertex v[]; };
layout(buffer_reference, scalar) buffer Indices { ivec3 i[]; };

#ifndef COMPUTE_SHADER
struct RayPayload {
	bool hit;
	vec3 pos;
	vec3 normal;
	vec3 color;
	float diffuseThreshold;
	float reflectThreshold;
	float transparentThreshold;
	float refractionIndex;
};

#ifdef HIT_OR_MISS_SHADER
layout(location = 0) rayPayloadInEXT RayPayload rayPayload;
#else
layout(location = 0) rayPayloadEXT RayPayload rayPayload;
#endif

#endif
