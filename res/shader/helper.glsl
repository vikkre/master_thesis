#define PI 3.1415926538


struct RNG {
	float seed;
	float i;
};

RNG initRNG(uvec3 seed) {
	RNG rng;
	rng.seed = dot(vec3(seed), vec3(12.9898, 78.233, 38.4965));
	rng.i = 0.0;
	return rng;
}

float rand(inout RNG rng) {
	float value = fract(sin(rng.seed + rng.i) * 43758.5453);
	rng.i += 1.0;
	return value;
}

vec3 randomNormal(inout RNG rng) {
	float theta = 2.0 * PI * rand(rng);
	float u = 2.0 * rand(rng) - 1.0;
	float suu = sqrt(1 - u*u);

	return vec3(
		suu * cos(theta),
		suu * sin(theta),
		u
	);
}

vec3 randomNormalDirection(inout RNG rng, vec3 normal) {
	vec3 randVec = randomNormal(rng);
	if (dot(randVec, normal) < 0.0) randVec *= -1.0;
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

vec3 sphericalFibonacci(float i, float n) {
	float iPhi = i * sqrt(5.0) * 0.5 + 0.5 - 1.0;
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
