#include <cstdint>
#include <iostream>
#include "AddressBus.h"

class PPU {
private:
	AddressBus bus;
	uint8_t *WX, *WY;
	uint16_t backgroundFIFO;
	uint16_t spriteFIFO;
	uint8_t* LCDC;
public:
	PPU(AddressBus& bus);
};