#include "SharpLR35902.h"
#include "SDL.h"
#include "AddressBus.h"
#include <memory>
#include "Timer.h"
#include "PPU.h"
#undef main

int main() {
	AddressBus bus;
	SharpLR35902 cpu(bus);
	Timer timer(bus);
	PPU ppuUnit(bus);
	
	cpu.setPC(0x100);
	while (true) {
		bus.setCycles(0);
		cpu.tick();
		ppuUnit.tick();
		timer.tick();
	}
}













/*int main() {
	SDL_Window* window;
	SDL_Init(SDL_INIT_VIDEO);

	window = SDL_CreateWindow(
		"GBEmu",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		SDL_WINDOW_OPENGL);

	if (window == NULL) {
		printf("Could not create window: %s\n", SDL_GetError());
	}

	SDL_Delay(3000);

	SDL_Quit();
	return 0;
}*/