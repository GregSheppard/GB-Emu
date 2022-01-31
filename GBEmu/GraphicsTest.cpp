#include "GraphicsTest.h"

void GraphicsTest::tick() {
	int x = 0;
	int width = 8;
	for (int y = 0; y < 16; y+=2) {
		uint8_t lower = tile[y];
		uint8_t upper = tile[y + 1];
	}

}

void GraphicsTest::initSDL() {
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		std::cout << "FATAL ERROR: could not initalize SDL: " << SDL_GetError() << std::endl;
		throw std::runtime_error("FATAL ERROR: could not initalize SDL!");
	}

	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
		std::cout << "WARNING: Linear texture filtering not enabled!";
	}

	window = std::shared_ptr<SDL_Window>(SDL_CreateWindow("Gameboy Emulator", SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED, 640, 576, SDL_WINDOW_SHOWN),
		[](SDL_Window* window) {
			SDL_DestroyWindow(window);
			window = NULL;
		});

	if (!window) {
		std::cout << "FATAL ERROR: could not create SDL window: " << SDL_GetError() << std::endl;
		throw std::runtime_error("FATAL ERROR: could not create SDL window!");
	}

	renderer = std::shared_ptr<SDL_Renderer>(SDL_CreateRenderer(window.get(), -1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC),
		[](SDL_Renderer* renderer) {
			SDL_DestroyRenderer(renderer);
			renderer = NULL;
		});

	if (!renderer) {
		std::cout << "FATAL ERROR: could not initialize SDL renderer: " << SDL_GetError() << std::endl;
		throw std::runtime_error("FATAL ERROR: could not create SDL renderer!");
	}
}