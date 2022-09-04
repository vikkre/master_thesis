#extension GL_EXT_ray_tracing: enable
#extension GL_EXT_nonuniform_qualifier: enable
#extension GL_EXT_scalar_block_layout: enable
#extension GL_EXT_buffer_reference2: require
#extension GL_EXT_shader_explicit_arithmetic_types_int64: require


#define PI 3.1415926538

#define LIGHT_COLLECTION_INDEX_STACK_SIZE 2000
#define LIGHT_COLLECTION_DATA_ARRAY_SIZE 100


struct PointData {
	vec3 pos;
	vec3 color;
};

struct KDData {
	vec3 pos;
	vec3 color;
	uint direction;
	int leftIndex;
	int rightIndex;
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
	uint lightRayCount;
	uint lightJumpCount;
	uint visionJumpCount;
	float collectionDistance;
	uint visionRayPerPixelCount;
	float collectionDistanceShrinkFactor;
	uint lightCollectionCount;
	uint useCountLightCollecton;
} renderSettings;
layout(binding = 4, set = 0) buffer Count {uint c;} count;
layout(binding = 5, set = 0, scalar) buffer PD { PointData d[]; } pointData;
layout(binding = 6, set = 0, scalar) buffer KDD { KDData d[]; } kdData;
layout(binding = 7, set = 0, r32ui) uniform uimage2D rawImageRed;
layout(binding = 8, set = 0, r32ui) uniform uimage2D rawImageGreen;
layout(binding = 9, set = 0, r32ui) uniform uimage2D rawImageBlue;
layout(binding = 10, set = 0, rgba8) uniform image2D finalImage;

layout(buffer_reference, scalar) buffer Vertices { Vertex v[]; };
layout(buffer_reference, scalar) buffer Indices { ivec3 i[]; };

#ifndef COMPUTE_SHADER
struct RayPayload {
	bool miss;
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
	if (ndotd > 0.0) rIndex = 1.0 / rIndex;

	vec3 nParallel = ndotd * normal;
	vec3 nOrthogonal = direction - nParallel;
	float nOrthogonalLength2 = dot(nOrthogonal, nOrthogonal);
	float rIndex2 = rIndex*rIndex;

	if (rIndex2 > nOrthogonalLength2) {
		return normalize(nOrthogonal + (sqrt(rIndex2 - nOrthogonalLength2) / length(nParallel)) * nParallel);
	} else {
		return reflect(direction, normal);
	}
}
