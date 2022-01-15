#include <cstdint>
#include <iostream>
#include <vector>
#include "AddressBus.h"

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

class PPU {
private:
	AddressBus& bus;
	uint8_t* WX, * WY, * SCY, *SCX, *LY, *LYC;
	uint16_t backgroundFIFO;
	uint16_t spriteFIFO;
	uint8_t* LCDC;
	uint8_t* STAT;
	uint8_t* OAM[0x9F+1];

	int cycles;
	PPUState currentState;

	std::vector<Sprite> spriteBuffer;
public:
	PPU(AddressBus& bus);
	void tick();
	PPUState getState();
};