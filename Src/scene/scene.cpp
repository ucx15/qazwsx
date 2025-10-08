#include <fstream>
#include <iostream>
#include <string>
#include <cstdint>


#include "nlohmann_json/json.hpp" // downloaded from https://github.com/nlohmann/json

#include "scene.hpp"


using json = nlohmann::json;


// Constructors and Destructors
Scene::Scene() {
	sceneVertexCount = 0;
	sceneTriangleCount = 0;
	sceneObjectCount = 0;
	sceneVerticies = nullptr;
	sceneObjects = nullptr;
	name = "default";
}

Scene::~Scene() {
	this->unload();
}


// Methods
bool Scene::loadJSONScene(const char *filename) {
	this->unload();

	std::ifstream file(filename);
	if (!file.is_open()) {
		std::cerr << "Failed to open scene file: " << filename << std::endl;
		return false;
	}

	json data;
	try {
		file >> data;
	}
	catch (const json::parse_error& e) {
		std::cerr << "JSON parse error: " << e.what() << std::endl;
		return false;
	}

	std::cout << "Loading Scene: " << filename << "\n";

	sceneVertexCount = data.value("vertexCount", -1);
	sceneObjectCount = data.value("objectCount", -1);
	name = data.value("name", "default");

	if (sceneVertexCount <= 0 || sceneObjectCount <= 0) {
		std::cerr << "No vertex or triangle in scene file." << std::endl;
		return false;
	}

	std::cout << "\nVertices: " << sceneVertexCount << ", Objects: " << sceneObjectCount << "\n";

	// Validate counts
	if (data["vertices"].size() != sceneVertexCount*3) {
		std::cerr << "Vertex count mismatch in scene file." << std::endl;
		std::cerr << "Expected " << sceneVertexCount*3 << " values, got " << data["vertices"].size() << std::endl;
		return false;
	}

	if (data["objects"].size() != sceneObjectCount) {
		std::cerr << "Object count mismatch in scene file." << std::endl;
		std::cerr << "Expected " << sceneObjectCount << " objects, got " << data["objects"].size() << std::endl;
		return false;
	}

	// Validate arrays
	if ( !data["vertices"].is_array()) {
		std::cerr << "Invalid vertices format in scene file." << std::endl;
		return false;
	}

	if ( !data["objects"].is_array() ) {
		std::cerr << "Invalid objects format in scene file." << std::endl;
		return false;
	}

	// Load Vertices and Objects
	const auto& verts = data["vertices"];
	const auto& objs = data["objects"];

	sceneVerticies = new Vec3[sceneVertexCount];
	sceneObjects = new Object[sceneObjectCount];

	for (uint32_t i=0, j=0; i<sceneVertexCount; i++) {
		Vec3 &v = sceneVerticies[i];
		v.x = verts[j++];
		v.y = verts[j++];
		v.z = verts[j++];
	}

	for (uint32_t i=0; i<sceneObjectCount; i++) {
		auto objData = objs[i];
		Object &obj = sceneObjects[i];

		obj.mesh = new Mesh();
		Mesh &mesh = *obj.mesh;

		auto objName = objData.value("name", "");
		obj.name = objName;
		std::cout << "\nLoading Object: '" << objName << "'\n";

		uint32_t vertexCount = objData.value("vertexCount", -1);
		uint32_t indexCount = objData.value("indexCount", -1);
		uint32_t triangleCount = objData.value("triangleCount", -1);
		obj.id = i;

		if (vertexCount <= 0 || indexCount <= 0 || triangleCount <= 0) {
			std::cerr << "No vertex or triangle in the object: '" << objName << "'\n";
			return false;
		}


		// Validate counts
		if (objData["indices"].size() != indexCount) {
			std::cerr << "Index count mismatch in scene file." << std::endl;
			std::cerr << "Expected " << indexCount*3 << " values, got " << objData["indices"].size() << std::endl;
			return false;
		}

		// Validate arrays
		if ( !objData["indices"].is_array()) {
			std::cerr << "Invalid indices format in scene file." << std::endl;
			return false;
		}

		mesh.vertexCount = vertexCount;
		mesh.indexCount = indexCount;
		mesh.triangleCount = triangleCount;
		std::cout
			<< "  Mesh Vertex Count: " << mesh.vertexCount
			<< ", Mesh Index Count: " << mesh.indexCount
			<< ", Mesh Triangle Count: " << mesh.triangleCount
			<< std::endl;


		// Load the indices
		const auto &indices = objData["indices"];
		mesh.indices = new uint32_t[mesh.indexCount];

		for (uint32_t j=0; j<mesh.indexCount; j++) {
			mesh.indices[j] = indices[j];
		}

		sceneTriangleCount += triangleCount;
		std::cout << "  Loaded Object: '" << obj.name << "' successfully.\n";
	}

	file.close();
	std::cout << "\nScene loaded successfully.\n\n";
	return true;

}

void Scene::unload() {
	delete [] sceneVerticies;
	sceneVerticies = nullptr;
	sceneVertexCount = 0;

	sceneTriangleCount = 0;

	delete [] sceneObjects;
	sceneObjects = nullptr;
	sceneObjectCount = 0;
}
