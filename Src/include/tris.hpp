#pragma once
#include "vec.hpp"

class Tris3D {
public:
	Vec3 v1, v2, v3;

public:
	// Ctors and Dtors
	Tris3D();
	Tris3D(Vec3 a, Vec3 b, Vec3 c);
	~Tris3D();

	// Methods
	Vec3 getCenter();
};


class Tris2D {
public:
	Vec2 v1, v2, v3;

	public:
// Ctors and Dtors
	Tris2D();
	Tris2D(Vec2 a, Vec2 b, Vec2 c);
	~Tris2D();

	// Methods
	Vec2 getCenter();
};


// 2D Integer Triangle : x0, y0, x1, y1, x2, y2
class Tris2D_i {
public:
	int x0, y0, x1, y1, x2, y2;
};
