#pragma once

// #include <string>
#include <cstdint>

#include "../math/vec.hpp"
// #include "tris.hpp"


class Mesh {
	// Constructors / Destructors
	public:
		Mesh();
		~Mesh();

	// Attributes
	public:
		uint32_t vertexCount;
		uint32_t indexCount;
		uint32_t triangleCount;

		uint32_t *indices;

	// Methods
	public:

};

