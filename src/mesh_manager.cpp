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
	InputParser parser(filename);
	parser.parse();

	for (unsigned int i = 0; i < parser.size(); ++i) {
		const InputEntry& entry = parser.getInputEntry(i);

		if (entry.name == "Probes") {
			probeData.probeCount = entry.getVector<3, unsigned int>("probeCount");
			probeData.totalProbeCount = probeData.probeCount[0] * probeData.probeCount[1] * probeData.probeCount[2];
			probeData.probeStartCorner = entry.getVector<3, float>("probeStartCorner");
			probeData.betweenProbeDistance = entry.getVector<3, float>("betweenProbeDistance");

			// for (unsigned int x = 0; x < probeData.probeCount[0]; ++x) {
			// 	for (unsigned int y = 0; y < probeData.probeCount[1]; ++y) {
			// 		for (unsigned int z = 0; z < probeData.probeCount[2]; ++z) {
			// 			Vector3f xyz = Vector3f({(float) x, (float) y, (float) z});
			// 			Vector3f pos;
			// 			for (unsigned int vi = 0; vi < 3; ++vi) {
			// 				pos[vi] = xyz[vi] * probeData.betweenProbeDistance[vi] + probeData.probeStartCorner[vi];
			// 			}

			// 			Mesh* mesh = getMesh("ball.obj");
			// 			GraphicsObject* obj = new GraphicsObject(device, mesh, pos);
			// 			constexpr float s = 0.1f;
			// 			obj->scale = Vector3f({s, s, s});
			// 			obj->color = Vector3f({1.0f, 0.0f, 0.0f});
			// 			createdObjects.push_back(obj);
			// 		}
			// 	}
			// }

			continue;
		}

		// if (!entry.keyExists("lightSource")) continue;

		Mesh* mesh = getMesh(entry.name);
		Vector3f pos = entry.getVector<3, float>("position");
		GraphicsObject* obj = new GraphicsObject(device, mesh, pos);

		if (entry.keyExists("color"))    obj->color    = entry.getVector<3, float>("color");
		if (entry.keyExists("scale"))    obj->scale    = entry.getVector<3, float>("scale");
		if (entry.keyExists("rotation")) obj->rotation = entry.getRotation("rotation");

		if (entry.keyExists("diffuse"))         obj->diffuseWeight          = entry.get<float>("diffuse");
		if (entry.keyExists("reflect"))         obj->reflectWeight          = entry.get<float>("reflect");
		if (entry.keyExists("transparent"))     obj->transparentWeight      = entry.get<float>("transparent");
		if (entry.keyExists("refractionIndex")) obj->rtData.refractionIndex = entry.get<float>("refractionIndex");

		if (entry.keyExists("move")) {
			obj->move = true;
			obj->moveStopPos = Vector3f({
				entry.get<float>("move", 0),
				entry.get<float>("move", 1),
				entry.get<float>("move", 2)
			});
			float dist = obj->moveStartPos.distance(obj->moveStopPos);
			obj->moveSpeed = entry.get<float>("move", 3) / dist;
		}

		if (entry.keyExists("rotate")) {
			obj->rotate = true;
			obj->rotationAxis = Vector3f({
				entry.get<float>("rotate", 0),
				entry.get<float>("rotate", 1),
				entry.get<float>("rotate", 2)
			});
			obj->rotationSpeed = entry.get<float>("rotate", 3);
		}

		if (entry.keyExists("lightSource")) {
			obj->rtData.lightSource = true;
			obj->rtData.lightStrength = entry.get<float>("lightSource", 0);
			createdLightSources.push_back(obj);
		} else {
			obj->rtData.lightSource = false;
		}

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
