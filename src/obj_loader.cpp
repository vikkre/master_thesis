#include "obj_loader.h"

#include "graphic/mesh.h"

#define OBJ_PATH std::string("../res/obj/")
#define MTL_PATH std::string("../res/obj/")

ObjLoader::ObjLoader()
:points(), texcoords(), normals(), faces(),
materials(), usedMaterial(nullptr) {
	std::pair<std::string, ObjLoader::Material> new_insert("none", ObjLoader::Material());
	materials.insert(new_insert);
}

ObjLoader::~ObjLoader() {
	
}

void ObjLoader::load(const std::string& name) {
	std::ifstream file(OBJ_PATH + name);
	if (!file.is_open()) {
		SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Could not load OBJ file %s", name.c_str());
		return;
	}
	std::string line;

	while (std::getline(file, line)) {
		std::istringstream line_stream(line);
		std::string mode;

		std::getline(line_stream, mode, ' ');

		if (mode == "mtllib") {
			std::string name;
			std::getline(line_stream, name);
			loadMtl(name);
		} else if (mode == "usemtl") {
			std::string name;
			std::getline(line_stream, name);
			if (materials.count(name) > 0) {
				usedMaterial = &materials[name];
			} else {
				usedMaterial = &materials["none"];
			}
		} else if (mode == "v") {
			points.push_back(loadPoint(line_stream));
		} else if (mode == "vt") {
			texcoords.push_back(loadTexcoord(line_stream));
		} else if (mode == "vn") {
			normals.push_back(loadNormal(line_stream));
		} else if (mode == "f") {
			faces.push_back(loadFace(line_stream));
		} 
	}

	file.close();
}

Vector3f ObjLoader::loadPoint(std::istringstream& line_stream) {
	std::string line;
	Vector3f value;

	std::getline(line_stream, line, ' ');
	value[0] = std::atof(line.c_str());
	std::getline(line_stream, line, ' ');
	value[1] = std::atof(line.c_str());
	std::getline(line_stream, line);
	value[2] = std::atof(line.c_str());

	return value;
}

Vector2f ObjLoader::loadTexcoord(std::istringstream& line_stream) {
	std::string line;
	Vector2f value;

	std::getline(line_stream, line, ' ');
	value[0] = std::atof(line.c_str());
	std::getline(line_stream, line);
	value[1] = 1.0f - std::atof(line.c_str());

	return value;
}

Vector3f ObjLoader::loadNormal(std::istringstream& line_stream) {
	std::string line;
	Vector3f value;

	std::getline(line_stream, line, ' ');
	value[0] = std::atof(line.c_str());
	std::getline(line_stream, line, ' ');
	value[1] = std::atof(line.c_str());
	std::getline(line_stream, line);
	value[2] = std::atof(line.c_str());

	return value;
}

std::vector<ObjLoader::Vertex> ObjLoader::loadFace(std::istringstream& line_stream) {
	std::vector<ObjLoader::Vertex> vertices;
	std::string line;

	while (std::getline(line_stream, line, ' ')) {
		std::istringstream face_stream(line);

		vertices.push_back(loadVertex(face_stream));
	}

	return vertices;
}

ObjLoader::Vertex ObjLoader::loadVertex(std::istringstream& line_stream) {
	std::string line;
	Vertex vertex;

	std::getline(line_stream, line, '/');
	vertex.point = std::atoi(line.c_str());
	std::getline(line_stream, line, '/');
	vertex.texcoord = std::atoi(line.c_str());
	std::getline(line_stream, line);
	vertex.normal = std::atoi(line.c_str());

	return vertex;
}

void ObjLoader::loadMtl(const std::string& name) {
	std::ifstream file(MTL_PATH + name);
	if (!file.is_open()) {
		SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "Could not load MTL file %s", name.c_str());
		return;
	}

	std::string line;
	ObjLoader::Material* current = nullptr;

	while (std::getline(file, line)) {
		std::istringstream line_stream(line);
		std::string mode;

		std::getline(line_stream, mode, ' ');

		if (mode == "newmtl") {
			std::string name;
			std::getline(line_stream, name);
			std::pair<std::string, ObjLoader::Material> new_insert(name, ObjLoader::Material());
			current = &materials.insert(new_insert).first->second;
			current->name = name;
		} else if (mode == "map_Kd") {
			std::string imageName;
			std::getline(line_stream, imageName);
			current->imageName = imageName;
		}
	}

	file.close();
}

Mesh* ObjLoader::get_mesh(const Device* device) const {
	Mesh* mesh = new Mesh(device);

	std::map<ObjLoader::Vertex, unsigned int> unique_vertices;
	for (std::vector<ObjLoader::Vertex> face: faces) {
		for (ObjLoader::Vertex vertex: face) {
			if (unique_vertices.count(vertex) == 0) {
				Vector3f point = points.at(vertex.point - 1);
				Vector3f normal = normals.at(vertex.normal - 1);
				mesh->addPoint(point, normal);
				unsigned int position = mesh->vertices.size() - 1;

				unique_vertices.insert(std::pair<ObjLoader::Vertex, unsigned int>(vertex, position));
			}
		}
	}

	for (std::vector<ObjLoader::Vertex> face: faces) {
		for (unsigned int i = 2; i < face.size(); ++i) {
			mesh->addIndex(Vector3u({
				unique_vertices.find(face.at(    0))->second,
				unique_vertices.find(face.at(i - 1))->second,
				unique_vertices.find(face.at(i    ))->second
			}));
		}
	}

	mesh->init();

	return mesh;
}
