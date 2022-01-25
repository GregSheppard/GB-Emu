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

struct Tile {
	uint8_t lower;
	uint8_t upper;
	Tile(uint8_t low, uint8_t up) : lower(low), upper(up) {};
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
	uint8_t* VRAMPointers[0x3FFF + 1];

	uint8_t background[256][256] = { 0 };
	uint8_t window[256][256] = { 0 };

	int cycles;
	int WINDOW_LINE_COUNTER;
	PPUState currentState;

	std::vector<Sprite> spriteBuffer;
public:
	PPU(AddressBus& bus);
	void tick();
	PPUState getState();
	void setState(PPUState state);
	bool getLCDCFlag(int bit);
	void setLCDCFlag(int bit);
	uint8_t VRAM(uint16_t address);

	uint8_t fetchBackgroundTileNumber(int xpos);
	uint8_t fetchWindowTileNumber(int xpos);
	Tile fetchTile(uint8_t tileNumber);
};