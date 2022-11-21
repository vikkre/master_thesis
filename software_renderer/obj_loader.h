#ifndef OBJ_LOADER_H
#define OBJ_LOADER_H

#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>

#include "init_exception.h"

#include "graphic/mesh.h"
#include "math/vector.h"

class Device;
class Mesh;

class ObjLoader {
	public:
		struct Vertex {
			unsigned int pos;
			unsigned int texcoord;
			unsigned int normal;

			bool operator<(const Vertex& other) const {
				if (pos < other.pos) return true;
				if (pos > other.pos) return false;
				if (texcoord < other.texcoord) return true;
				if (texcoord > other.texcoord) return false;
				if (normal < other.normal) return true;
				return false;
			}
		};

		struct Material {
			Material(): name(), imageName("no_texture.png") {}

			std::string name;
			std::string imageName;
		};

		ObjLoader(const std::string& basepath);
		~ObjLoader();

		void load(const std::string& name);
		Mesh* get_mesh() const;

		std::vector<Vector3f> points;
		std::vector<Vector2f> texcoords;
		std::vector<Vector3f> normals;
		std::vector<std::vector<Vertex>> faces;
		std::map<std::string, Material> materials;
		Material* usedMaterial;
	private:
		Vector3f loadPoint(std::istringstream& line_stream);
		Vector2f loadTexcoord(std::istringstream& line_stream);
		Vector3f loadNormal(std::istringstream& line_stream);
		std::vector<Vertex> loadFace(std::istringstream& line_stream);
		Vertex loadVertex(std::istringstream& line_stream);

		void loadMtl(const std::string& name);

		std::string basepath;
};

#endif
