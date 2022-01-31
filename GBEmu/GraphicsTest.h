#pragma once
#include <cstdint>
#include <iostream>
#include <vector>
#include "AddressBus.h"
#include <memory>
#include <SDL.h>

enum class PPUState {
	HBlank, VBlank, OAMScan, Drawing
};

struct Sprite {
	uint8_t Ypos;
	uint8_t Xpos;
	uint8_t tileNum;
	uint8_t spriteFlags;
	Sprite(uint8_t y, uint8_t x, uint8_t tile, uint8_t flags)
		:Ypos(y), Xpos(x), tileNum(tile), spriteFlags(flags) {};
};

struct Tile {
	uint8_t lower;
	uint8_t upper;
	Tile(uint8_t low, uint8_t up) : lower(low), upper(up) {};
};

class GraphicsTest {
private:
	std::shared_ptr<SDL_Renderer> renderer;
	std::shared_ptr<SDL_Window> window = NULL;

	uint8_t tile[16] = {
	0x0, 0x0, 0x3c, 0x3c, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x3c, 0x0, 0x0
	};

	uint8_t framebuffer[160 * 144 * 3] = { 0 };

public:
	GraphicsTest();
	void tick();

	void initSDL();
};