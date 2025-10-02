#pragma once

#include "SDL3/SDL.h"

#include "vec.hpp"
#include "color.hpp"
#include "tris.hpp"
#include "rect.hpp"
#include "scene.hpp"
#include "surface.hpp"


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

		Scene enScene;         			// Scene object
		int enVxCount;
		int enTriCount;

		Vec3 *enVerticies; 				// Holds the 3D verticies of the scene
		Tris3D_ref *enTrisRefBuffer; 	// Holds the triangle references to be rasterized
		Tris2D *enTrisProjectedBuffer; 	// Holds the triangles that are projected


		// Rendering Stuff
		bool isRunning;
		float deltaTime;

		glm::mat4 projMat;

	public:
		Engine();
		~Engine();
		void pipeline(const char *filename);

	private:
		void SDLSetup();
		void SDLDestroy();

		void handleEvents();

		void engineSetup();
		void engineDestroy();

		void loadScene(const char *filename);
		void transform();
		void sortGeometry();
		void project();
		void render();
		void rasterize();
};
