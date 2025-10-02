#include "mesh.hpp"

// Constructors and Destructors
Mesh::Mesh() {
	vertexCount = 0;
	indexCount = 0;
	indices  = nullptr;
}

Mesh::~Mesh() {
	vertexCount = 0;

	delete [] indices;
	indices = nullptr;
	indexCount = 0;
}
