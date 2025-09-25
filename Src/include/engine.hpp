#pragma once

#include "SDL3/SDL.h"

#include "vec.hpp"
#include "color.hpp"
#include "surface.hpp"
#include "rect.hpp"
#include "tris.hpp"
#include "scene.hpp"


class Engine {

	private:
		// SDL Stuff
		SDL_Window *SDLWindow;
		SDL_Renderer *SDLRenderer;
		SDL_Texture *SDLTexture;
		SDL_Event SDLEvent;

		// Engine Stuff
		uint32_t *enTextureBuffer;  // intermediate Buffer for writing to SDLTexture
		Color *enBuffer;            // Array of pixels
		Surface enSurface;

		int enVxCount;
		int enTriCount;

		Vec3 *enVerticies; 		// Holds the 3D verticies of the scene
		Tris3D *enTrisBuffer; 	// Holds the triangles to be rasterized

		Tris2D *enTrisProjectedBuffer; 	// Holds the triangles that are projected
		int *enTriIndex; 	    // Holds the index buffer for the triangles

		Vec3 *enSSVerticies; 	// Holds the projected 2D verticies of the scene

		Scene enScene;         // Scene object

		// Rendering Stuff
		bool isRunning;
		float deltaTime;

		glm::mat4 projMat;

	public:
		Engine();
		~Engine();
		void pipeline();

	private:
		void SDLSetup();
		void SDLDestroy();

		void handleEvents();

		void engineSetup();
		void engineDestroy();

		void loadScene(const char *filename);
		void project();
		void sortGeometry(); 	// in Descending order of depth
		void render();
		void rasterize();
};
