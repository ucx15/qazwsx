#include <fstream>
#include <iostream>

#include "nlohmann_json/json.hpp" // downloaded from https://github.com/nlohmann/json
#include "scene.hpp"


// Constructors and Destructors
Scene::Scene() {
	sceneVertexCount = 0;
	sceneTriangleCount = 0;

	sceneVerticies = nullptr;
	sceneTris = nullptr;
	sceneTriIndex = nullptr;
}
Scene::~Scene() {
	this->unload();
}

// Methods
void Scene::loadJSONScene(const char *filename) {
	this->unload();

	std::ifstream file(filename);
	if (!file.is_open()) {
		std::cerr << "Failed to open scene file: " << filename << std::endl;
		return;
	}

	using json = nlohmann::json;
	json data;
	try {
		file >> data;
	}
	catch (const json::parse_error& e) {
		std::cerr << "JSON parse error: " << e.what() << std::endl;
		exit(EXIT_FAILURE);
	}

	std::cout << "Loading Scene: " << filename << "\n";
	sceneVertexCount = data.value("vertexCount", -1);
	sceneTriangleCount = data.value("triangleCount", -1);

	if (sceneVertexCount <= 0 || sceneTriangleCount <= 0) {
		std::cerr << "Invalid vertex or triangle count in scene file." << std::endl;
		exit(EXIT_FAILURE);
	}

	std::cout << "Vertex Count: " << sceneVertexCount << "\n";
	std::cout << "Triangle Count: " << sceneTriangleCount << "\n";

	sceneVerticies = new Vec3[sceneVertexCount];
	sceneTris = new Tris3D[sceneTriangleCount];
	sceneTriIndex = new int[3*sceneTriangleCount];


	if (data["vertices"].size() != sceneVertexCount) {
		std::cerr << "Vertex count mismatch in scene file." << std::endl;
		exit(EXIT_FAILURE);
	}

	if (data["triangles"].size() != sceneTriangleCount) {
		std::cerr << "Triangle count mismatch in scene file." << std::endl;
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < sceneVertexCount; i++) {
		const auto &v = data["vertices"][i];
		auto x = v.value("x", 0.0f);
		auto y = v.value("y", 0.0f);
		auto z = v.value("z", 0.0f);

		sceneVerticies[i].x = x;
		sceneVerticies[i].y = y;
		sceneVerticies[i].z = z;
	}

	for (int i = 0, j = 0; i < sceneTriangleCount; i++) {
		const auto &t = data["triangles"][i];
		auto i1 = t.value("v1", -1);
		auto i2 = t.value("v2", -1);
		auto i3 = t.value("v3", -1);

		if (i1 < 0 || i1 >= sceneVertexCount ||
		    i2 < 0 || i2 >= sceneVertexCount ||
		    i3 < 0 || i3 >= sceneVertexCount) {
			std::cerr << "Invalid vertex index in triangle " << i << std::endl;
			exit(EXIT_FAILURE);
		}

		sceneTris[i].v1.x = sceneVerticies[i1].x;
		sceneTris[i].v1.y = sceneVerticies[i1].y;
		sceneTris[i].v1.z = sceneVerticies[i1].z;

		sceneTris[i].v2.x = sceneVerticies[i2].x;
		sceneTris[i].v2.y = sceneVerticies[i2].y;
		sceneTris[i].v2.z = sceneVerticies[i2].z;

		sceneTris[i].v3.x = sceneVerticies[i3].x;
		sceneTris[i].v3.y = sceneVerticies[i3].y;
		sceneTris[i].v3.z = sceneVerticies[i3].z;

		sceneTriIndex[j++] = i1;
		sceneTriIndex[j++] = i2;
		sceneTriIndex[j++] = i3;
	}

	file.close();
	std::cout << "Scene loaded successfully.\n";
}

void Scene::unload() {
	if (sceneVerticies) {
		free(sceneVerticies);
		sceneVerticies = nullptr;
	}
	if (sceneTris) {
		free(sceneTris);
		sceneTris = nullptr;
	}
	if (sceneTriIndex) {
		free(sceneTriIndex);
		sceneTriIndex = nullptr;
	}

	sceneVertexCount = 0;
	sceneTriangleCount = 0;
}
