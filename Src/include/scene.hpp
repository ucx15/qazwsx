//  JSON Scene file loader

#pragma once
#include "vec.hpp"
#include "tris.hpp"


class Scene {

public:
	// Scene Data
	int sceneVertexCount;
	int sceneTriangleCount;
	int sceneIndexBufferSize;

	Vec3 *sceneVerticies;    	// Raw 3D Vec Verticies
	int *sceneIndexBuffer; 		// Index Buffer
	Tris3D_ref *sceneTrisRef;	// 3D Reference Triangles buffer

public:
	Scene();
	~Scene();

// Methods
public:
	void loadJSONScene(const char *filename);
	void unload();
};
