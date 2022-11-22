#include "graphics_object.h"


GraphicsObject::GraphicsObject(const Mesh* mesh, const Vector3f& position)
:scale({1.0f, 1.0f, 1.0f}), rotation(), position(position),
color(Vector3f({1.0f, 1.0f, 1.0f})), lightSource(false), lightStrength(0.0f),
diffuseWeight(1.0f), reflectWeight(0.0f), transparentWeight(0.0f), refractionIndex(1.0f),
indices(), vertices(), mesh(mesh), objectMatrix() {}

GraphicsObject::~GraphicsObject() {}

void GraphicsObject::init() {
	indices = mesh->indices;
	vertices.resize(mesh->vertices.size());

	for (size_t i = 0; i < mesh->vertices.size(); ++i) {
		Vector4f pos = expandVector(mesh->vertices[i].pos, 1.0f);
		Vector4f normal = expandVector(mesh->vertices[i].normal, 0.0f);

		Matrix4f mat = getMatrix();
		pos = mat * pos;
		normal = mat * normal;

		vertices[i].pos = cutVector(pos);
		vertices[i].normal = cutVector(normal);
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
