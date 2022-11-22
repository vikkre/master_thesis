#include "graphics_object.h"


GraphicsObject::GraphicsObject(const Mesh* mesh, const Vector3f& position)
:scale({1.0f, 1.0f, 1.0f}), rotation(), position(position),
color(Vector3f({1.0f, 1.0f, 1.0f})), lightSource(false), lightStrength(0.0f),
diffuseWeight(1.0f), reflectWeight(0.0f), transparentWeight(0.0f), refractionIndex(1.0f),
vertices(), mesh(mesh), objectMatrix(), triangles(), aabbMin(), aabbMax() {}

GraphicsObject::~GraphicsObject() {}

void GraphicsObject::init() {
	vertices.resize(mesh->vertices.size());

	for (size_t i = 0; i < mesh->vertices.size(); ++i) {
		Vector4f pos = expandVector(mesh->vertices[i].pos, 1.0f);
		Vector4f normal = expandVector(mesh->vertices[i].normal, 0.0f);

		Matrix4f mat = getMatrix();
		pos = mat * pos;
		normal = mat * normal;

		vertices[i].pos = cutVector(pos);
		vertices[i].normal = cutVector(normal);

		if (i == 0) {
			aabbMin = vertices[i].pos;
			aabbMax = vertices[i].pos;
		} else {
			for (size_t a = 0; a < 3; ++a) {
				aabbMin[a] = std::min(aabbMin[a], vertices[i].pos[a]);
				aabbMax[a] = std::max(aabbMax[a], vertices[i].pos[a]);
			}
		}
	}

	triangles.reserve(mesh->indices.size() / 3);
	for (unsigned int i = 0; i < mesh->indices.size(); i += 3) {
		unsigned int v0 = mesh->indices[i+0];
		unsigned int v1 = mesh->indices[i+1];
		unsigned int v2 = mesh->indices[i+2];

		triangles.emplace_back(
			Vector3u({v0, v1, v2}),
			vertices[v0].pos,
			vertices[v1].pos,
			vertices[v2].pos
		);
	}

	float totalWeight = diffuseWeight + reflectWeight + transparentWeight;
	diffuseThreshold = diffuseWeight / totalWeight;
	reflectThreshold = diffuseThreshold + (reflectWeight / totalWeight);
	transparentThreshold = reflectThreshold + (transparentWeight / totalWeight);
}

Matrix4f GraphicsObject::getMatrix() const {
	Matrix4f objectMatrix;

	objectMatrix *= getScaleMatrix(scale);
	objectMatrix *= rotation.getMatrix();
	objectMatrix *= getTranslationMatrix(position);

	return objectMatrix;
}

bool GraphicsObject::traceRay(const Vector3f& rayOrigin, const Vector3f& rayDirection, Vector3f& hitPos, const Triangle*& currentTriangle, float& minDistance) const {
	for (size_t a = 0; a < 3; ++a) {
		float invD = 1.0f / rayDirection[a];
		float t0 = (aabbMin[a] - rayOrigin[a]) * invD;
		float t1 = (aabbMax[a] - rayOrigin[a]) * invD;

		if (invD < 0.0f) {
			if (t1 > t0) return false;
		} else {
			if (t1 < t0) return false;
		}
	}
	
	bool hit = false;
	for (const Triangle& triangle: triangles) {
		Vector3f currentHitPos;
		if (triangle.rayIntersects(rayOrigin, rayDirection, currentHitPos)) {
			float currentDistance = rayOrigin.distanceSquared(currentHitPos);
			if (currentDistance < minDistance) {
				hitPos = currentHitPos;
				minDistance = currentDistance;
				currentTriangle = &triangle;
				hit = true;
			}
		}
	}
	return hit;
}
