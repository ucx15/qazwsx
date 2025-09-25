#include "SDL3/SDL.h"
#include "engine.hpp"
#include "settings.hpp"

// #define TRACK_MEMORY    // Can be used to Track Allocated and Deallocated memory
#include "utils.hpp"


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

	MEM_DEALLOC(enTriIndex,      3*enTriCount);
	MEM_DEALLOC(enTrisProjectedBuffer, enTriCount);
	MEM_DEALLOC(enTrisBuffer, enTriCount);
	MEM_DEALLOC(enSSVerticies,   enVxCount);
	MEM_DEALLOC(enVerticies,     enVxCount);

	MEM_DEALLOC(enBuffer, 		 W*H);
	MEM_DEALLOC(enTextureBuffer, W*H);

	enScene.unload();
}

void Engine::loadScene(const char *filename) {
	enScene.loadJSONScene(filename);

	enVxCount = enScene.sceneVertexCount;
	enTriCount = enScene.sceneTriangleCount;

	MEM_ALLOC(enVerticies, Vec3, enVxCount);
	MEM_ALLOC(enSSVerticies, Vec3, enVxCount);

	MEM_ALLOC(enTrisBuffer, Tris3D, enTriCount);
	MEM_ALLOC(enTrisProjectedBuffer, Tris2D, enTriCount);

	MEM_ALLOC(enTriIndex, int, 3*enTriCount);

	// Point Scene Data to Engine Buffers

	enVerticies = enScene.sceneVerticies;
	enTrisBuffer = enScene.sceneTris;
	enTriIndex = enScene.sceneTriIndex;

}


// Sorting the geometry in Descending order of depth by Tris3D::getCenter().z
void Engine::sortGeometry() {
	std::sort(enTrisBuffer, enTrisBuffer + enTriCount, [](Tris3D &a, Tris3D &b) {
		return a.getCenter().z < b.getCenter().z;
	});
}

// Rendering Methods
// void Engine::project() {
// 	for (int i=0; i<enVxCount; i++) {
// 		Vec3 &in = enVerticies[i];     // Input Vector
// 		Vec3 &out = enSSVerticies[i];  // Output Vector

// 		Vec4 intr = projMat * Vec4(in, 1.0f);

//         // Normalize intr coordinates
//         if (intr.w != 0) {
//             out.x = intr.x/intr.w;
//             out.y = intr.y/intr.w;
//             out.z = intr.z/intr.w;
//         }

// 		// Normal Space to Screen Space conversion
// 		// (-1, 1)  -- x2 ->  (0, 2)  -- /2 ->  (0, 1)  -- xS ->  (0, S)
// 		out.x = W * (1.f+out.x)/2.f;
// 		out.y = H * (1.f-out.y)/2.f;
// 	}
// }

void Engine::project() {
	for (int i=0; i<enTriCount; i++) {
		Tris3D &t3d = enTrisBuffer[i];

		Vec3 pts[3] = { t3d.v1, t3d.v2, t3d.v3 };
		Vec2 ptsOut[3] = { Vec2(), Vec2(), Vec2() };

		for(int j=0; j<3; j++){
			Vec3 &in = pts[j];     // Input Vector
			Vec2 &out = ptsOut[j];  // Output Vector

			Vec4 intr = projMat * Vec4(in, 1.0f);

			// Normalize intr coordinates
			if (intr.w != 0) {
				out.x = intr.x/intr.w;
				out.y = intr.y/intr.w;
				// out.z = intr.z/intr.w;
			}

			// Normal Space to Screen Space conversion
			// (-1, 1)  -- x2 ->  (0, 2)  -- /2 ->  (0, 1)  -- xS ->  (0, S)
			out.x = W * (1.f+out.x)/2.f;
			out.y = H * (1.f-out.y)/2.f;
		}

		enTrisProjectedBuffer[i] = Tris2D( ptsOut[0], ptsOut[1], ptsOut[2] );
	}
}

void Engine::rasterize() {
	// Rendering Triangles from ss_points buffer
	enSurface.fill(COLOR_BLACK);

	// // Drawing Triangles
	// for (int i=0; i<enTriCount*3; i+=3) {
	// 	Vec3 &a = enSSVerticies[ enTriIndex[i]   ];
	// 	Vec3 &b = enSSVerticies[ enTriIndex[i+1] ];
	// 	Vec3 &c = enSSVerticies[ enTriIndex[i+2] ];
	// 	enSurface.fillTris(a,b,c, COLOR_BLUE);
	// 	enSurface.drawTris(a,b,c, COLOR_WHITE, 1);
	// }

	// Drawing Triangles
	for (int i=0; i<enTriCount; i++) {
		Tris2D &t = enTrisProjectedBuffer[i];
		Vec2 &a = t.v1;
		Vec2 &b = t.v2;
		Vec2 &c = t.v3;

		// enSurface.fillTris(a,b,c, COLOR_BLUE);
		// enSurface.drawTris(a,b,c, COLOR_WHITE, 1);

		Tris2D_i t_render(a.x, a.y, b.x, b.y, c.x, c.y);

		enSurface.fillTris(t_render, COLOR_BLUE);
		enSurface.drawTris(t_render, COLOR_WHITE, 1);

	}

	// Drawing Verticies
	// for (int i=0; i<enVxCount; i++) {
	// 	enSurface.fillCircle(enSSVerticies[i], 4, COLOR_RED);
	// }
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


void Engine::pipeline() {
	// Engine Class Startup code
	this->engineSetup();

	// Loading Scene into Memory
	this->loadScene("Scenes/default.json");


	TIME_PT tPtSortGeo1, tPtSortGeo2, tPtProject1, tPtProject2, tPtRaster1, tPtRaster2;


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
	auto tSortuS   = TIME_DUR(tPtSortGeo2, tPtSortGeo1);
	auto tProjectuS = TIME_DUR(tPtProject2, tPtProject1);
	auto tRasteruS  = TIME_DUR(tPtRaster2, tPtRaster1);
	std::cout << "\nSort\t " << tSortuS << " us\tProject\t " << tProjectuS << " us\tRaster\t " << tRasteruS << " us\n";


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
