#include "UI.h"

UI::UI() {
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		std::cout << "FATAL ERROR: could not initalize SDL: " << SDL_GetError() << std::endl;
		throw std::runtime_error("FATAL ERROR: could not initalize SDL!");
	}

	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
		std::cout << "WARNING: Linear texture filtering not enabled!";
	}

	window = std::shared_ptr<SDL_Window>(SDL_CreateWindow("Gameboy Emulator", SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN),
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

	SDL_SetRenderDrawColor(renderer.get(), 0xFF, 0xFF, 0xFF, 0xFF);

	while (true) {
		draw();
	}
}

void UI::draw() {
	SDL_SetRenderDrawColor(renderer.get(), 255, 255, 255, 255);
	SDL_RenderClear(renderer.get());

	SDL_Rect r;
	r.x = 50;
	r.y = 50;
	r.w = 2;
	r.h = 2;

	SDL_SetRenderDrawColor(renderer.get(), 0, 0, 0, 255);
	SDL_RenderFillRect(renderer.get(), &r);
	SDL_RenderPresent(renderer.get());
}