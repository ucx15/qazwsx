#include <iostream>
#include "engine.hpp"
#include "SDL3/SDL_main.h"

int main(int argc, char *argv[]) {
	(void) argc;
	(void) argv;

	Engine LiRasterEngine = Engine();
	LiRasterEngine.pipeline();

	return EXIT_SUCCESS;
}
