#include "SharpLR35902.h"
#include "SDL.h"
#undef main
#include <stdio.h>
#include <memory>
#include "GraphicsTest.h"
#include "Timer.h"

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define FREQ 4194304

int main() {
	//UI i;
	AddressBus bus;
	SharpLR35902 cpu(bus);
	Timer timer(bus);
	GraphicsTest gtest(bus);
	
	cpu.setPC(0x100);
	while (true) {
		bus.setCycles(0);
		cpu.tick();
		timer.tick();
		gtest.tick();
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