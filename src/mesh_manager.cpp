#include "mesh_manager.h"

inline bool ends_with(std::string const & value, std::string const & ending) {
	if (ending.size() > value.size()) return false;
	return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}


MeshManager::MeshManager(Device* device, const std::string& basepath)
:device(device), basepath(basepath), meshes(), createdObjects() {}

MeshManager::~MeshManager() {
	for (GraphicsObject* obj: createdObjects) delete obj;
	for (std::pair<std::string, Mesh*> mesh: meshes) delete mesh.second;
}

Mesh* MeshManager::getMesh(const std::string& name) {
	if (meshes.count(name) > 0) {
		return meshes[name];
	} else {
		Mesh* mesh = nullptr;
		if      (ends_with(name, ".obj")) mesh = loadObj(name);
		else if (ends_with(name, ".stl")) mesh = loadStl(name);
		meshes[name] = mesh;
		return mesh;
	}
}

void MeshManager::createObjectsFromFile(const std::string filename) {
	InputParser parser(basepath + filename);
	parser.parse();

	for (unsigned int i = 0; i < parser.size(); ++i) {
		const InputEntry& entry = parser.getInputEntry(i);

		Mesh* mesh = getMesh(entry.name);
		Vector3f pos = entry.getVector3f("position");
		GraphicsObject* obj = new GraphicsObject(device, mesh, pos);

		if (entry.keyExists("color")) obj->color = entry.getVector3f("color");
		if (entry.keyExists("scale")) obj->scale = entry.getVector3f("scale");
		if (entry.keyExists("rotation")) obj->rotation = entry.getRotation("rotation");

		createdObjects.push_back(obj);
	}
}

Mesh* MeshManager::loadObj(const std::string& filename) {
	ObjLoader blockLoader(basepath);
	blockLoader.load(filename);
	Mesh* mesh = blockLoader.get_mesh(device);
	return mesh;
}

Mesh* MeshManager::loadStl(const std::string& filename) {
	STLLoader loader(basepath);
	loader.load(filename);
	Mesh* mesh = loader.get_mesh(device);
	return mesh;
}
