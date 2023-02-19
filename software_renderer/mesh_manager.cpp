#include "mesh_manager.h"

inline bool ends_with(std::string const & value, std::string const & ending) {
	if (ending.size() > value.size()) return false;
	return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}


MeshManager::MeshManager(const std::string& basepath)
:basepath(basepath), meshes(), createdObjects() {}

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
		mesh->init();
		meshes[name] = mesh;
		return mesh;
	}
}

void MeshManager::createObjectsFromFile(const std::string filename) {
	InputParser parser(filename);
	parser.parse();

	for (unsigned int i = 0; i < parser.size(); ++i) {
		const InputEntry& entry = parser.getInputEntry(i);

		if (entry.name == "Probes") continue;

		Mesh* mesh = getMesh(entry.name);
		Vector3f pos = entry.getVector<3, float>("position");
		GraphicsObject* obj = new GraphicsObject(mesh, pos);

		if (entry.keyExists("color"))    obj->color    = entry.getVector<3, float>("color");
		if (entry.keyExists("scale"))    obj->scale    = entry.getVector<3, float>("scale");
		if (entry.keyExists("rotation")) obj->rotation = entry.getRotation("rotation");

		if (entry.keyExists("diffuse"))         obj->diffuseWeight     = entry.get<float>("diffuse");
		if (entry.keyExists("reflect"))         obj->reflectWeight     = entry.get<float>("reflect");
		if (entry.keyExists("transparent"))     obj->transparentWeight = entry.get<float>("transparent");
		if (entry.keyExists("refractionIndex")) obj->refractionIndex   = entry.get<float>("refractionIndex");

		if (entry.keyExists("lightSource")) {
			obj->lightSource = true;
			obj->lightStrength = entry.get<float>("lightSource", 0);
		}

		createdObjects.push_back(obj);
	}
}

std::vector<GraphicsObject*> MeshManager::getCreatedObjects() const {
	return createdObjects;
}

Mesh* MeshManager::loadObj(const std::string& filename) {
	ObjLoader blockLoader(basepath);
	blockLoader.load(filename);
	Mesh* mesh = blockLoader.get_mesh();
	return mesh;
}

Mesh* MeshManager::loadStl(const std::string& filename) {
	STLLoader loader(basepath);
	loader.load(filename);
	Mesh* mesh = loader.get_mesh();
	return mesh;
}
