#include <iostream>

#include "object.hpp"


// Constructors and Destructors
Object::Object() {
	name = "";
	id = rand();
	mesh = nullptr;
}

Object::~Object() {
	delete mesh;
	mesh = nullptr;
}
