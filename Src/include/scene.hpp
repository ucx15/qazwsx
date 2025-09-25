//  JSON Scene file loader

#pragma once
#include "vec.hpp"
#include "tris.hpp"


class Scene {

public:
	// Scene Data
	int sceneVertexCount;
	int sceneTriangleCount;

	Vec3 *sceneVerticies;    // Raw 3D Vec Verticies
	Tris3D *sceneTris;		// Raw 3D Triangle list
	int *sceneTriIndex;      // Index Buffer for the triangles

public:
	Scene();
	~Scene();

// Methods
public:
	void loadJSONScene(const char *filename);
	void unload();
};
