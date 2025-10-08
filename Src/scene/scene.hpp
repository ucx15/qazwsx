//  JSON Scene file loader

#pragma once

#include <string>
#include <cstdint>

#include "../math/vec.hpp"
#include "object.hpp"

class Scene {

public:
	// Scene Data
	uint32_t sceneVertexCount;	// Vertex Count
	uint32_t sceneTriangleCount;	// Triangle Count
	uint32_t sceneObjectCount;	// Object Count

	Vec3 *sceneVerticies;    	// Raw Verticies
	Object *sceneObjects;		// Objects in the scene

	std::string name;			// Scene Name

public:
	Scene();
	~Scene();

// Methods
public:
	bool loadJSONScene(const char *filename);
	void unload();
};
