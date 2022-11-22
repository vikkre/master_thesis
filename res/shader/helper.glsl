#define PI 3.1415926538


struct RNG {
	uint seed;
	uint a;
	uint c;
	uint m;
};

float rand(inout RNG rng) {
	rng.seed = (rng.a * rng.seed + rng.c) % rng.m;
	return float(rng.seed) / float(rng.m);
}

RNG initRNG(uvec3 seed) {
	RNG rng;
	rng.a = 6237;
	rng.c = 10697;
	rng.m = 21023;
	rng.seed = (625 + seed.x * 7444 + seed.y * 1045 + seed.z * 13336) % rng.m;
	return rng;
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
		ndotd = dot(normal, direction);
	} else {
		rIndex = 1.0 / rIndex;
	}

	float angle = sin(acos(ndotd)) * rIndex;
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

float square_length(vec3 v) {
	return v.x*v.x + v.y*v.y + v.z*v.z;
}
