#pragma once


#include <vector>
#include <string>

#include "graphic/device.h"
#include "graphic/mesh.h"
#include "graphic/graphics_object.h"

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

		std::vector<GraphicsObject*> getCreatedObjects() const { return createdObjects; }

	private:
		Mesh* loadObj(const std::string& filename);
		Mesh* loadStl(const std::string& filename);

		Device* device;
		std::string basepath;

		std::vector<Mesh*> loadedMeshes;
		std::vector<GraphicsObject*> createdObjects;

		Mesh* block;
		Mesh* ball;
		Mesh* upperTeeth;
		Mesh* lowerTeeth;

	public:
		void createBlocksAndBall(float reflect);
		void createTeeth(float reflect);

		void createCornellBox();
		void createCornellBoxBlocks(float reflect);
};
