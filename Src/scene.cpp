#include <fstream>
#include <iostream>

#include "nlohmann_json/json.hpp" // downloaded from https://github.com/nlohmann/json
#include "scene.hpp"


// Constructors and Destructors
Scene::Scene() {
	sceneVertexCount = 0;
	sceneTriangleCount = 0;
	sceneIndexBufferSize = 0;

	sceneVerticies = nullptr;
	sceneTrisRef = nullptr;
	sceneIndexBuffer = nullptr;
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
	sceneIndexBufferSize = data.value("triangleCount", -1) * 3;

	if (sceneVertexCount <= 0 || sceneTriangleCount <= 0) {
		std::cerr << "No vertex or triangle in scene file." << std::endl;
		exit(EXIT_FAILURE);
	}

	std::cout << "Vertices: " << sceneVertexCount << ", Triangles: " << sceneTriangleCount << std::endl;

	sceneVerticies = new Vec3[sceneVertexCount];
	sceneTrisRef = new Tris3D_ref[sceneTriangleCount];
	sceneIndexBuffer = new int[sceneIndexBufferSize];

	if (data["vertices"].size() != sceneVertexCount*3) {
		std::cerr << "Vertex count mismatch in scene file." << std::endl;
		std::cerr << "Expected " << sceneVertexCount*3 << " values, got " << data["vertices"].size() << std::endl;
		exit(EXIT_FAILURE);
	}

	if (data["triangles"].size() != sceneTriangleCount*3) {
		std::cerr << "Triangle count mismatch in scene file." << std::endl;
		std::cerr << "Expected " << sceneTriangleCount*3 << " indices, got " << data["triangles"].size() << std::endl;
		exit(EXIT_FAILURE);
	}

	data["vertices"].is_array();
	data["triangles"].is_array();

	if (!data["vertices"].is_array() || !data["triangles"].is_array()) {
		std::cerr << "Invalid vertices or triangles format in scene file." << std::endl;
		exit(EXIT_FAILURE);
	}

	const auto& verts = data["vertices"];
	const auto& tris = data["triangles"];

	float x, y, z;      // for vertex coords
	int i1, i2, i3; // for triangle vertex indices

	for (int i = 0, j = 0; i < sceneVertexCount; i++) {
		Vec3 &tmp = sceneVerticies[i];

		x = verts[j++];
		y = verts[j++];
		z = verts[j++];

		tmp.x = x;
		tmp.y = y;
		tmp.z = z;
	}

	for (int i=0, j=0, k=0; i < sceneTriangleCount; i++) {
		i1 = tris[j++];
		i2 = tris[j++];
		i3 = tris[j++];

		if (i1 < 0 || i1 >= sceneVertexCount ||
		    i2 < 0 || i2 >= sceneVertexCount ||
		    i3 < 0 || i3 >= sceneVertexCount) {
			std::cerr << "Invalid vertex index in triangle " << i << std::endl;
			std::cerr << "Expected indices between 0 and " << sceneVertexCount - 1 << ", got "
			          << i1 << ", " << i2 << ", " << i3 << std::endl;

			exit(EXIT_FAILURE);
		}

		Tris3D_ref &tri = sceneTrisRef[i];

		tri.v1 = &sceneVerticies[i1];
		tri.v2 = &sceneVerticies[i2];
		tri.v3 = &sceneVerticies[i3];

		sceneIndexBuffer[k++] = i1;
		sceneIndexBuffer[k++] = i2;
		sceneIndexBuffer[k++] = i3;
	}

	file.close();
	std::cout << "Scene loaded successfully.\n";
}

void Scene::unload() {
	if (sceneVerticies) {
		delete [] sceneVerticies;
	}
	if (sceneTrisRef) {
		delete [] sceneTrisRef;
	}

	if (sceneIndexBuffer) {
		delete [] sceneIndexBuffer;
	}

	sceneVertexCount = 0;
	sceneTriangleCount = 0;
	sceneIndexBufferSize = 0;
}
