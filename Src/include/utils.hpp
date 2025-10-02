# pragma once

#include <chrono>
#include <cstdint>
#include <iostream>

#include "vec.hpp"
#include "color.hpp"


using namespace std::chrono;


// Constants
const float PI = 3.14159f;

// Custom Colors
const Color COLOR_WHITE(1.f);
const Color COLOR_BLACK(0.f);
const Color COLOR_GRAY(0.5f);
const Color COLOR_RED(1.f, 0.f, 0.f);
const Color COLOR_GREEN(0.f, 1.0f, 0.f);
const Color COLOR_BLUE(0.f, 0.f, 1.f);
const Color COLOR_YELLOW(1.f, 1.f, 0.f);
const Color COLOR_PURPLE(1.f, 0.f, 1.f);


// Macro Expressions

#ifndef TRACK_MEMORY
	#define MEM_ALLOC(ptr, dType, N) {ptr = new dType[(N)];}
	#define MEM_DEALLOC(ptr, N) delete[] ptr;

#else
	static uint64_t memoryAllocated = 0;  // in Bytes

	// Should NOT be used Explicitly!
	#define _LOG_MEM_USING(t) std::cout << "Mem" << t << '\t' << memoryAllocated/1024.f << " kB\n"

	#define MEM_ALLOC(ptr, dType, N) {			\
		ptr = new dType[(N)];					\
		_LOG_MEM_USING("++");					\
		memoryAllocated += sizeof(dType)*(N);	\
	}											\

	#define MEM_DEALLOC(ptr, N) {				\
		delete[] ptr;							\
		memoryAllocated -= sizeof(*ptr)*(N);	\
		_LOG_MEM_USING("--");					\
	}											\

#endif


// #define _CLOCK_TYPE steady_clock
#define _CLOCK_TYPE high_resolution_clock

#define TIME_PT _CLOCK_TYPE::time_point
#define TIME_NOW() _CLOCK_TYPE::now()
#define TIME_DUR(b, a) duration_cast<microseconds>(b - a).count()
#define TIME_DUR_NS(b, a) duration_cast<nanoseconds>(b - a).count()

#define LOG_VEC3(v) std::cout << (v).x << ' ' << (v).y << ' ' << (v).z << '\n';
#define LOG_VEC4(v) std::cout << (v).x << ' ' << (v).y << ' ' << (v).z << ' ' << (v).w << '\n';
#define LOG_COLOR(c) std::cout << (c).r << ' ' << (c).g << ' ' << (c).b << '\n';

#define randf() ((float) pcg32_random_r() / UINT32_MAX)

// components of vector are between 0 and 1
#define randVec3() ( Vec3(randf(), randf(), randf()) )

#define randColor() ( Color(randf(), randf(), randf()) )

// components of vector are between -1 and 1
#define randBiVec3() ( Vec3(2*randf() -1, 2*randf() -1, 2*randf() -1) )

// components of vector are between a and b
#define randBwVec3(a, b) ( Vec3(glm::max(a, b*randf), glm::max(a, b*randf), glm::max(a, b*randf)) )


// Functions
uint32_t pcg32_random_r();
Vec3 randVec3onSphere(Vec3 normal);
