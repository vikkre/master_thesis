#ifndef OBJ_LOADER_H
#define OBJ_LOADER_H

#include <SDL2/SDL.h>

#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>

#include "math/vector.h"

class Device;
class Mesh;

class ObjLoader {
	public:
		struct Vertex {
			unsigned int point;
			unsigned int texcoord;
			unsigned int normal;

			bool operator<(const Vertex& other) const {
				if (point < other.point) return true;
				if (point > other.point) return false;
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

		ObjLoader();
		~ObjLoader();

		void load(const std::string& name);
		Mesh* get_mesh(const Device* device) const;

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
};

#endif
