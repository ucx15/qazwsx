#include <iostream>
#include "core/engine.hpp"
#include "SDL3/SDL_main.h"

int main(int argc, char *argv[]) {
	// (void) argc;
	// (void) argv;

	if (argc < 2) {
		std::cerr << "Error: No scene file provided." << std::endl;
		std::cerr << "Usage: \n\tqazwsx <scene_file.json>" << std::endl;
		return EXIT_FAILURE;
	}

	const char *filename = argv[1];

	Engine LiRasterEngine = Engine();
	LiRasterEngine.pipeline(filename);

	return EXIT_SUCCESS;
}
