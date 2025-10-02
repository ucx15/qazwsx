#pragma once

#include <string>
#include <cstdint>

#include "mesh.hpp"

class Object {
	// Constructors / Destructors
	public:
		Object();
		~Object();

	// Attributes
	public:
		std::string name;
		uint32_t id;
		Mesh *mesh;

	// Methods
	public:

};
