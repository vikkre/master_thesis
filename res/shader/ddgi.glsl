#extension GL_EXT_ray_tracing: enable
#extension GL_EXT_nonuniform_qualifier: enable
#extension GL_EXT_scalar_block_layout: enable
#extension GL_EXT_buffer_reference2: require
#extension GL_EXT_shader_explicit_arithmetic_types_int64: require


#define PI 3.1415926538


struct Surfel {
	vec3 rayDirection;
	vec3 hitRadiance;
	float hitDistance;
	bool hit;
};

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


layout(binding = 0, set = 0) uniform accelerationStructureEXT topLevelAS;
layout(binding = 1, set = 0, scalar) buffer ObjectProperties_ { ObjectProperties l[]; } objProps;
layout(binding = 2, set = 0, scalar) uniform GlobalData {
	mat4 viewInverse;
	mat4 projInverse;
	mat4 view;
	mat4 proj;
} globalData;
layout(binding = 3, set = 0, scalar) uniform RenderSettings {
	vec3 backgroundColor;
	vec3 lightPosition;
	uint lightJumpCount;
	uint visionJumpCount;
	float betweenProbeDistance;
	uint singleDirectionProbeCount;
	uint totalProbeCount;
	uint perProbeRayCount;
	float maxProbeRayDistance;
	uint probeSampleSideLength;
	float depthSharpness;
	float normalBias;
	float crushThreshold;
	uint linearBlending;
	float energyPreservation;
	float texelGetProbeDirectionFactor;
	float texelGetNormalFactor;
} renderSettings;
layout(binding = 4, set = 0, scalar) buffer SB { Surfel s[]; } surfels;
layout(binding = 5, set = 0, rgba8) uniform image2D irradianceBuffer;
layout(binding = 6, set = 0, rgba8) uniform image2D depthBuffer;
layout(binding = 7, set = 0) uniform sampler2D irradianceSampler;
layout(binding = 8, set = 0) uniform sampler2D depthSampler;
layout(binding = 9, set = 0, rgba8) uniform image2D finalImage;

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

#ifdef RAYGEN_SHADER
layout(location = 0) rayPayloadEXT RayPayload rayPayload;

float getShade(vec3 pos, vec3 normal) {
	vec3 toLight = renderSettings.lightPosition - pos;
	vec3 normToLight = normalize(toLight);
	float lightStrenth = dot(normToLight, normal);
	if (lightStrenth <= 0.0) return 0.0;
	float distToLight = length(toLight);

	RayPayload tmp = rayPayload;

	uint rayFlags = gl_RayFlagsOpaqueEXT;
	uint cullMask = 0xFF;
	float tmin = 0.001;
	traceRayEXT(topLevelAS, rayFlags, cullMask, 0, 0, 0, pos, tmin, normToLight, distToLight, 0);

	if (rayPayload.hit) return 0.0;

	rayPayload = tmp;

	return dot(normalize(renderSettings.lightPosition - pos), normal);

}
#else
layout(location = 0) rayPayloadInEXT RayPayload rayPayload;
#endif

#endif

float rand(vec3 co, int i, int v){
	return fract(sin(dot(co, vec3(12.9898, 78.233, 38.4965)) * (v + 1.0) + i) * 43758.5453);
}

vec3 randomNormal(vec3 co, int i, int vStart) {
	float theta = 2.0 * PI * rand(co, i, vStart);
	float u = 2.0 * rand(co, i, vStart+1) - 1.0;
	float suu = sqrt(1 - u*u);

	return vec3(
		suu * cos(theta),
		suu * sin(theta),
		u
	);
}

vec3 randomNormalDirection(vec3 co, int i, int vStart, vec3 normal) {
	vec3 randVec = randomNormal(co, i, vStart);

	if (dot(randVec, normal) < 0.0) {
		randVec *= -1;
	}

	return randVec;
}

vec3 customRefract(vec3 direction, vec3 normal, float rIndex) {
	float ndotd = dot(normal, direction);
	if (ndotd > 0.0) {
		normal = -normal;
	} else {
		rIndex = 1.0 / rIndex;
	}

	float angle = sin(acos(dot(direction, normal))) * rIndex;
	if (-1.0 < angle && angle < 1.0)
		return refract(direction, normal, rIndex);
	else
		return reflect(direction, normal);
}

#define MOD_PHI sqrt(5.0) * 0.5 + 0.5 - 1.0
vec3 sphericalFibonacci(float i, float n) {
	float iPhi = i * MOD_PHI;
	float phi = 2.0 * PI * (iPhi - floor(iPhi));
	float cosTheta = 1.0 - (2.0 * i + 1.0) * (1.0 / n);
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

	return vec3(
		cos(phi) * sinTheta,
		sin(phi) * sinTheta,
		cosTheta
	);
}


vec2 signNotZero(vec2 v) {
	return vec2(
		v.x >= 0.0 ? 1.0 : -1.0,
		v.y >= 0.0 ? 1.0 : -1.0
	);
}

vec2 octEncode(vec3 v) {
	float l1norm = abs(v.x) + abs(v.y) + abs(v.z);
	vec2 result = v.xy * (1.0 / l1norm);
	if (v.z < 0.0) {
		result = (1.0 - abs(result.yx)) * signNotZero(result.xy);
	}
	return result;
}


/** Returns a unit vector. Argument o is an octahedral vector packed via octEncode,
    on the [-1, +1] square*/
vec3 octDecode(vec2 o) {
	vec3 v = vec3(o.x, o.y, 1.0 - abs(o.x) - abs(o.y));
	if (v.z < 0.0) {
		v.xy = (1.0 - abs(v.yx)) * signNotZero(v.xy);
	}
	return normalize(v);
}

vec3 lerp(vec3 v0, vec3 v1, vec3 t) {
	return v0 + t * (v1 - v0);
}
