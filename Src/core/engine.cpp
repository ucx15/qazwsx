#include <iostream>
#include <vector>
#include <cmath>
#include <numbers>

#include "SDL3/SDL.h"
#include "engine.hpp"
#include "settings.hpp"

// #define TRACK_MEMORY    // Can be used to Track Allocated and Deallocated memory
#include "../utils/utils.hpp"


/*
Engine Class handles SDL Setup and Deinitialization itself
while RenderEngine Setup and destruction is explicitly
handled by Engine::pipeline()
*/


// Constructors and Destructors
Engine::Engine() {
	this->SDLSetup();
}

Engine::~Engine() {
	this->SDLDestroy();
}


// SDL Methods (Window Management & Event Handling)
void Engine::SDLSetup() {
	if( !SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) ) {
		SDL_Log("SDL_Init failed: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	SDLWindow = SDL_CreateWindow("LiRaster", W, H, 0);
	if ( !SDLWindow ) {
		SDL_Log("SDL_CreateWindow creation failed: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}

    SDLRenderer =  SDL_CreateRenderer(SDLWindow, NULL);
	if ( !SDLRenderer ) {
		SDL_Log("SDL_CreateRenderer creation failed: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	SDLTexture = SDL_CreateTexture(SDLRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, W, H);
	if ( !SDLTexture ) {
		SDL_Log("SDL_CreateTexture failed: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}
}

void Engine::SDLDestroy() {
	SDL_DestroyTexture(SDLTexture);
	SDL_DestroyRenderer(SDLRenderer);
	SDL_DestroyWindow(SDLWindow);
	SDL_Quit();
}

void Engine::handleEvents() {
	while (SDL_PollEvent(&SDLEvent)) {
		if (SDLEvent.type == SDL_EVENT_QUIT) {
			isRunning = false;
		}
	}
}


// Engine Methods (Setup, Destruction and Scene Loading)
void Engine::engineSetup() {
	MEM_ALLOC(enTextureBuffer, uint32_t, W*H);
	MEM_ALLOC(enBuffer, Color,W*H);

	enSurface = Surface(enBuffer, W, H);
	projMat = glm::perspective(glm::radians(AOV), ASR, EPSILON, FAR_CLIP);

	isRunning = true;
}

void Engine::engineDestroy() {

	MEM_DEALLOC(enTrisRefBuffer, enTriCount);
	MEM_DEALLOC(enTrisProjectedBuffer, enTriCount);
	MEM_DEALLOC(enVerticies, enVxCount);

	MEM_DEALLOC(enBuffer, 		 W*H);
	MEM_DEALLOC(enTextureBuffer, W*H);

}

void Engine::loadScene(const char *filename) {
	enScene.loadJSONScene(filename);

	enVxCount = enScene.sceneVertexCount;
	enTriCount = enScene.sceneTriangleCount;

	MEM_ALLOC(enVerticies, Vec3, enVxCount);
	MEM_ALLOC(enTrisRefBuffer, Tris3D_ref, enTriCount);
	MEM_ALLOC(enTrisProjectedBuffer, Tris2D, enTriCount);

	// Point Scene Data to Engine Buffers
	for (int i=0; i<enVxCount; i++) {
		enVerticies[i].x = enScene.sceneVerticies[i].x;
		enVerticies[i].y = enScene.sceneVerticies[i].y;
		enVerticies[i].z = enScene.sceneVerticies[i].z;
	}

	// Set up the engine's triangle references to point to engine vertices
	for (uint32_t i=0; i<enScene.sceneObjectCount; i++) {
		Mesh &mesh = *enScene.sceneObjects[i].mesh;

		for (uint32_t j=0, k=0; j<mesh.triangleCount; j++) {
			Tris3D_ref &tRef = enTrisRefBuffer[j];
			tRef.v1 = &enVerticies[ mesh.indices[k++] ];
			tRef.v2 = &enVerticies[ mesh.indices[k++] ];
			tRef.v3 = &enVerticies[ mesh.indices[k++] ];
		}
	}

	enScene.unload();
}


// Geometry Methods (Transformations, Sorting, Projection)
// Transformation
void Engine::transform() {

	// TODO: Replace with proper transformation matrices

	Vec3 translation(0.f, 0.f, -3.f); 	// Move everything away from camera a bit

	// float rotationX = glm::radians(45.0f); // in radians
	float rotationY = glm::radians(10.0f); // in radians
	// float rotationZ = glm::radians(10.0f); // in radians

	float rotationX = 0.f; // in radians
	// float rotationY = 0.f; // in radians
	float rotationZ = 0.f; // in radians


	glm::mat4 rotXMat = glm::rotate(glm::mat4(1.0f), rotationX, Vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 rotYMat = glm::rotate(glm::mat4(1.0f), rotationY, Vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 rotZMat = glm::rotate(glm::mat4(1.0f), rotationZ, Vec3(0.0f, 0.0f, 1.0f));

	std::cout << "Translation: " << translation.x << ", " << translation.y << ", " << translation.z << std::endl;
	std::cout << "Rotation: " << rotationX << ", " << rotationY << ", " << rotationZ << std::endl;

	// Applying transformations to all verticies
	for (int i=0; i<enVxCount; i++) {
		Vec3 &v = enVerticies[i];

		// x Rotation
		if (rotationX != 0)	{
			Vec4 rx = rotXMat * Vec4(v, 1.0f);
			v = Vec3(rx);
		}

		// y Rotation
		if (rotationY != 0)	{
			Vec4 ry = rotYMat * Vec4(v, 1.0f);
			v = Vec3(ry);
		}

		// z Rotation
		if (rotationZ != 0)	{
			Vec4 rz = rotZMat * Vec4(v, 1.0f);
			v = Vec3(rz);
		}

		// Translation
		v += translation;
	}

}

// Sorting the geometry in Descending order of depth by Tris3D::getCenter().z
void Engine::sortGeometry() {
	std::sort(enTrisRefBuffer, enTrisRefBuffer + enTriCount, [](Tris3D_ref &a, Tris3D_ref &b) {
		return a.getCenter().z < b.getCenter().z;
	});
}

// TODO: Handle out of screen projected points
// Projects 3D Reference Triangles to 2D Triangles
void Engine::project() {
	for (int i=0; i<enTriCount; i++) {
		const Tris3D_ref tRef = enTrisRefBuffer[i];
		Tris2D &out = enTrisProjectedBuffer[i];

		Vec3 *inVecs[3] = {tRef.v1, tRef.v2, tRef.v3};
		Vec2 *outVecs[3] = {&out.v1, &out.v2, &out.v3};

		for (int j=0; j<3; j++) {
			Vec3 in_vec = *inVecs[j];     // Input Vector
			Vec2 *out_vec = outVecs[j];   // Output Vector

			Vec4 intr = projMat * Vec4(in_vec, 1.0f);

			// Normalize intr coordinates
			if (intr.w != 0) {
				out_vec->x = intr.x/intr.w;
				out_vec->y = intr.y/intr.w;
			}

			// Normal Space to Screen Space conversion
			// (-1, 1)  -- x2 ->  (0, 2)  -- /2 ->  (0, 1)  -- xS ->  (0, S)
			out_vec->x = W * (1.f+out_vec->x)/2.f;
			out_vec->y = H * (1.f-out_vec->y)/2.f;
		}
	}
}


// Rendering Methods
void Engine::rasterize() {
	// Rendering Triangles from ss_points buffer
	enSurface.fill(COLOR_BLACK);

	Vec3 light_dir = glm::normalize( Vec3(-1.f, -1.f, -1.f) );

	// Drawing Triangles
	for (int i=0; i<enTriCount; i++) {
		Tris2D &tRender = enTrisProjectedBuffer[i];
		Vec2 &a = tRender.v1;
		Vec2 &b = tRender.v2;
		Vec2 &c = tRender.v3;

		// Fill Triangle
		Vec3 normal = enTrisRefBuffer[i].getNormal();
		float light_intensity = glm::dot(normal, -light_dir);
		Color fillColor = COLOR_BLUE * light_intensity;

		// enSurface.fillTris(tRender, COLOR_BLUE);
		// enSurface.fillTris(tRender, fillColor);
		enSurface.fillTris(tRender, normal);

		// Draw Triangle
		// enSurface.drawTris(tRender, COLOR_WHITE, 1);

		// Draw Verticies
		// enSurface.fillCircle(a, 2, COLOR_WHITE);
		// enSurface.fillCircle(b, 2, COLOR_WHITE);
		// enSurface.fillCircle(c, 2, COLOR_WHITE);
	}

	// NOTE: Debug Center Lines
	enSurface.drawLine(0, H/2, W-1, H/2, COLOR_RED, 1);
	enSurface.drawLine(W/2, 0, W/2, H-1, COLOR_GREEN, 1);

	// Tonemapping
	enSurface.tonemap();
}

void Engine::render() {
	// Copying data to 32 bit buffer
	enSurface.toU32Surface(enTextureBuffer);

	// Copying data to VRAM
	SDL_UpdateTexture(SDLTexture, NULL, enTextureBuffer, W*4);
	if ( !SDLTexture ) {
		SDL_Log("SDL_CreateTextureFromSurface failed: %s", SDL_GetError());
		return;
	}

	// Presenting to Display device
	SDL_RenderClear(SDLRenderer);
	SDL_RenderTexture(SDLRenderer, SDLTexture, NULL, NULL);
	SDL_RenderPresent(SDLRenderer);
}


void Engine::pipeline(const char *filename) {
	// Engine Class Startup code
	this->engineSetup();

	// Loading Scene into Memory
	// this->loadScene("Scenes/default.json");
	this->loadScene(filename);


	TIME_PT tPtTransform1, tPtTransform2, tPtSortGeo1, tPtSortGeo2, tPtProject1, tPtProject2, tPtRaster1, tPtRaster2;

	// Transformation
	// TODO: Fix it with parameters properly
	// NOTE: This is just for testing purposes
	tPtTransform1 = TIME_NOW();
	this->transform();
	tPtTransform2 = TIME_NOW();


	// Sorting Geometry
	tPtSortGeo1 = TIME_NOW();
	this->sortGeometry();
	tPtSortGeo2 = TIME_NOW();


	// Projection
	tPtProject1 = TIME_NOW();
	this->project();
	tPtProject2 = TIME_NOW();


	// Rasterization
	tPtRaster1 = TIME_NOW();
	this->rasterize();
	tPtRaster2 = TIME_NOW();

	// Logging
	auto tTransformuS = TIME_DUR(tPtTransform2, tPtTransform1);
	auto tSortuS   = TIME_DUR(tPtSortGeo2, tPtSortGeo1);
	auto tProjectuS = TIME_DUR(tPtProject2, tPtProject1);
	auto tRasteruS  = TIME_DUR(tPtRaster2, tPtRaster1);

	std::cout
		<< "\nTransform: " << tTransformuS << "/" << (tTransformuS/1E3F) << " \t"
		<< "Sort: "      << tSortuS        << "/" << (tSortuS/1E3F)	     << " \t"
		<< "Project: "   << tProjectuS     << "/" << (tProjectuS/1E3F)	 << " \t"
		<< "Raster: "    << tRasteruS      << "/" << (tRasteruS/1E3F)	 << " \t(us/ms)\n";


	float lastLogTime = 0.f;
	TIME_PT tPtRender1, tPtRender2, tDt1, tDt2;

	tDt1 = TIME_NOW();

	// Main Loop
	while (isRunning) {

		// Calculate delta time
		{
			tDt2 = TIME_NOW();
			deltaTime = TIME_DUR(tDt2, tDt1)/1E6F;
			tDt1 = TIME_NOW();
		}


		// Handle Events
		this->handleEvents();


		// Render
		tPtRender1 = TIME_NOW();
			this->render();
		tPtRender2 = TIME_NOW();

		lastLogTime += deltaTime;

		// Logs all the timings
		if ( lastLogTime>UPDATE_TIME ) {
			lastLogTime = 0.f;

			auto tRender = TIME_DUR(tPtRender2, tPtRender1);
			std::cout << "FPS " << 1/deltaTime << "\tRender  " << tRender/1E3F << " ms\tdt " << deltaTime*1E3F << " ms\n";
		}
	}


	TIME_PT tPtSave1, tPtSave2;
	// Save the Surface
	tPtSave1 = TIME_NOW();
	enSurface.savePNG("Out/img.png");
	tPtSave2 = TIME_NOW();

	uint64_t t_save_us   = TIME_DUR(tPtSave2, tPtSave1);
	std::cout << "\nSave\t " << t_save_us / 1000.f << " ms\n\n";


	// Engine Class Destructor Code
	this->engineDestroy();
}
