#pragma once


#include <vector>
#include <string>
#include <unordered_map>

#include "graphic/device.h"
#include "graphic/mesh.h"
#include "graphic/graphics_object.h"

#include "input_parser.h"
#include "stl_loader.h"
#include "obj_loader.h"

#include "math/vector.h"
#include "math/matrix.h"
#include "math/rotation.h"


class MeshManager {
	public:
		MeshManager(Device* device, const std::string& basepath);
		~MeshManager();

		void init();
		void initTeethMesh();
		Mesh* getMesh(const std::string& name);
		void createObjectsFromFile(const std::string filename);

		std::vector<GraphicsObject*> getCreatedObjects() const { return createdObjects; }
		std::vector<GraphicsObject*> getCreatedLightSources() const { return createdLightSources; }

	private:
		Mesh* loadObj(const std::string& filename);
		Mesh* loadStl(const std::string& filename);

		Device* device;
		std::string basepath;

		std::unordered_map<std::string, Mesh*> meshes;
		std::vector<GraphicsObject*> createdObjects;
		std::vector<GraphicsObject*> createdLightSources;
};
