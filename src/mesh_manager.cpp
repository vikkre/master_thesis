#include "mesh_manager.h"


MeshManager::MeshManager(Device* device, const std::string& basepath)
:device(device), basepath(basepath), loadedMeshes(), createdObjects(),
block(nullptr), ball(nullptr), upperTeeth(nullptr), lowerTeeth(nullptr) {}

MeshManager::~MeshManager() {
	for (GraphicsObject* obj: createdObjects) delete obj;
	for (Mesh* mesh: loadedMeshes) delete mesh;
}

void MeshManager::init() {
	block = loadObj("block.obj");
	ball  = loadObj("ball.obj");
}

void MeshManager::initTeethMesh() {
	lowerTeeth = loadStl("Lower.stl");
	upperTeeth = loadStl("Upper.stl");
}

Mesh* MeshManager::loadObj(const std::string& filename) {
	ObjLoader blockLoader(basepath);
	blockLoader.load(filename);
	Mesh* mesh = blockLoader.get_mesh(device);
	loadedMeshes.push_back(mesh);
	return mesh;
}

Mesh* MeshManager::loadStl(const std::string& filename) {
	STLLoader loader(basepath);
	loader.load(filename);
	Mesh* mesh = loader.get_mesh(device);
	loadedMeshes.push_back(mesh);
	return mesh;
}

void MeshManager::createBlocksAndBall(float reflect) {
	GraphicsObject* white = new GraphicsObject(device, ball, Vector3f({0.0f, 0.0f, 0.0f}));
	white->color = Vector3f({1.0f, 1.0f, 1.0f});
	createdObjects.push_back(white);
	white->diffuseWeight = 1.0f - reflect;
	white->reflectWeight = reflect;
	
	GraphicsObject* red = new GraphicsObject(device, block, Vector3f({0.0f, 0.0f, 3.0f}));
	red->color = Vector3f({1.0f, 0.0f, 0.0f});
	createdObjects.push_back(red);
	
	GraphicsObject* green = new GraphicsObject(device, block, Vector3f({-3.0f, 0.0f, 0.0f}));
	green->color = Vector3f({0.0f, 1.0f, 0.0f});
	createdObjects.push_back(green);
	
	GraphicsObject* blue = new GraphicsObject(device, block, Vector3f({0.0f, 0.0f, -3.0f}));
	blue->color = Vector3f({0.0f, 0.0f, 1.0f});
	createdObjects.push_back(blue);
	
	GraphicsObject* black = new GraphicsObject(device, block, Vector3f({3.0f, 0.0f, 0.0f}));
	black->color = Vector3f({0.1f, 0.1f, 0.1f});
	createdObjects.push_back(black);
}

void MeshManager::createTeeth(float reflect) {
	GraphicsObject* upper = new GraphicsObject(device, upperTeeth, Vector3f({0.0f, 0.0f, 0.0f}));
	upper->color = Vector3f({1.0f, 0.0f, 0.0f});
	createdObjects.push_back(upper);

	GraphicsObject* lower = new GraphicsObject(device, lowerTeeth, Vector3f({0.0f, 0.0f, 0.0f}));
	lower->color = Vector3f({0.0f, 1.0f, 0.0f});
	createdObjects.push_back(lower);
	lower->diffuseWeight = 1.0f - reflect;
	lower->reflectWeight = reflect;
}

void MeshManager::createCornellBox() {
	GraphicsObject* back = new GraphicsObject(device, block, Vector3f({-5.0f, 0.0f, 0.0f}));
	back->scale = Vector3f({0.1f, 5.0f, 5.0f});
	createdObjects.push_back(back);

	GraphicsObject* front = new GraphicsObject(device, block, Vector3f({25.0f, 0.0f, 0.0f}));
	front->scale = Vector3f({0.1f, 5.0f, 5.0f});
	createdObjects.push_back(front);

	GraphicsObject* top = new GraphicsObject(device, block, Vector3f({10.0f, 5.0f, 0.0f}));
	top->scale = Vector3f({15.0f, 0.1f, 5.0f});
	createdObjects.push_back(top);

	GraphicsObject* bottom = new GraphicsObject(device, block, Vector3f({10.0f, -5.0f, 0.0f}));
	bottom->scale = Vector3f({15.0f, 0.1f, 5.0f});
	createdObjects.push_back(bottom);

	GraphicsObject* red = new GraphicsObject(device, block, Vector3f({10.0f, 0.0f, -5.0f}));
	red->color = Vector3f({1.0f, 0.0f, 0.0f});
	red->scale = Vector3f({15.0f, 5.0f, 0.1f});
	createdObjects.push_back(red);
	
	GraphicsObject* green = new GraphicsObject(device, block, Vector3f({10.0f, 0.0f, 5.0f}));
	green->color = Vector3f({0.0f, 1.0f, 0.0f});
	green->scale = Vector3f({15.0f, 5.0f, 0.1f});
	createdObjects.push_back(green);
}

void MeshManager::createCornellBoxBlocks(float reflect) {
	GraphicsObject* smallBox = new GraphicsObject(device, block, Vector3f({2.0f, -3.5f, -2.0f}));
	smallBox->scale = Vector3f({1.5f, 1.5f, 1.5f});
	smallBox->rotation.set(Vector3f({0.0f, 1.0f, 0.0f}), 1.0f);
	smallBox->diffuseWeight = 1.0f - reflect;
	smallBox->reflectWeight = reflect;
	createdObjects.push_back(smallBox);

	GraphicsObject* bigBox = new GraphicsObject(device, block, Vector3f({-2.0f, -2.0f, 2.0f}));
	bigBox->scale = Vector3f({1.5f, 3.0f, 1.5f});
	bigBox->rotation.set(Vector3f({0.0f, 1.0f, 0.0f}), -1.0f);
	bigBox->diffuseWeight = 1.0f - reflect;
	bigBox->reflectWeight = reflect;
	createdObjects.push_back(bigBox);
}
