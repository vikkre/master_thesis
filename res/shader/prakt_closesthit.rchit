#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#define M_PI 3.1415926535897932384626433832795
#define MIN_LIGHT 0.0


struct ObjectProperties {
	mat4 model;
	vec3 color;
	uint64_t vertexAddress;
	uint64_t indexAddress;
	float diffuseThreshold;
	float reflectThreshold;
	float glossyThreshold;
	float transparentThreshold;
};

struct Vertex {
  vec3 pos;
  vec3 normal;
};

layout(binding = 0, set = 0) uniform accelerationStructureEXT topLevelAS;
layout(binding = 2, set = 0, scalar) uniform GlobalData {
	mat4 viewInverse;
	mat4 projInverse;
	mat4 view;
	mat4 proj;
	vec3 backgroundColor;
	vec3 lightPosition;
} globalData;
layout(binding = 3, set = 0, scalar) buffer ObjectProperties_ { ObjectProperties l[]; } objProps;
layout(buffer_reference, scalar) buffer Vertices { Vertex v[]; };
layout(buffer_reference, scalar) buffer Indices { ivec3 i[]; };

struct RayPayload {
	vec3 color;
	float distance;
	vec3 normal;
	float reflector;
};
layout(location = 0) rayPayloadInEXT RayPayload rayPayload;
layout(location = 2) rayPayloadEXT bool shadowed;
hitAttributeEXT vec2 attribs;

void main() {
	ObjectProperties obj = objProps.l[gl_InstanceID];
	Indices indices      = Indices(obj.indexAddress);
	Vertices vertices    = Vertices(obj.vertexAddress);

	const vec3 barycentricCoords = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);

	ivec3 ind = indices.i[gl_PrimitiveID];
	Vertex v0 = vertices.v[ind.x];
	Vertex v1 = vertices.v[ind.y];
	Vertex v2 = vertices.v[ind.z];

	vec3 pos = v0.pos * barycentricCoords.x + v1.pos * barycentricCoords.y + v2.pos * barycentricCoords.z;
	pos = (obj.model * vec4(pos, 1.0)).xyz;
	vec3 normal = v0.normal * barycentricCoords.x + v1.normal * barycentricCoords.y + v2.normal * barycentricCoords.z;
	normal = normalize(normal);

	vec3 lightVector = normalize(globalData.lightPosition);
	float dot_product = max(dot(lightVector, normal), MIN_LIGHT);

	rayPayload.distance = gl_RayTmaxEXT;
	rayPayload.normal = normal;
	rayPayload.reflector = obj.reflectThreshold;

	shadowed = true;
	vec3 origin = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;
	uint rayFlags = gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT;
	uint cullMask = 0xFF;
	float tmin = 0.001;
	float tmax = min(10000.0, distance(globalData.lightPosition, origin));

	traceRayEXT(topLevelAS, rayFlags, cullMask, 1, 0, 1, origin, tmin, lightVector, tmax, 2);

	if (shadowed) {
		rayPayload.color = obj.color * MIN_LIGHT;
	} else {
		rayPayload.color = obj.color * dot_product;
	}
}
