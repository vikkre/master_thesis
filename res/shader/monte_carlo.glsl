#extension GL_EXT_ray_tracing: enable
#extension GL_EXT_nonuniform_qualifier: enable
#extension GL_EXT_scalar_block_layout: enable
#extension GL_EXT_buffer_reference2: require
#extension GL_EXT_shader_explicit_arithmetic_types_int64: require


#include "helper.glsl"
#include "v1.glsl"


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

layout(set = 0, binding = 0, scalar) uniform RenderSettings {
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
layout(set = 0, binding = 1) buffer Count {uint c;} count;
layout(set = 0, binding = 2, scalar) buffer PD { PointData d[]; } pointData;
layout(set = 0, binding = 3, scalar) buffer KDD { KDData d[]; } kdData;
layout(set = 0, binding = 4, r32ui) uniform uimage2D rawImageRed;
layout(set = 0, binding = 5, r32ui) uniform uimage2D rawImageGreen;
layout(set = 0, binding = 6, r32ui) uniform uimage2D rawImageBlue;
layout(set = 0, binding = 7, rgba8) uniform image2D finalImage;
