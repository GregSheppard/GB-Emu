#include "PPU.h"

PPU::PPU(AddressBus& _bus) {
	bus = _bus;
	WY = bus.getRegister(0xFF4A);
	WX = bus.getRegister(0xFF4B);
	LCDC = bus.getRegister(0xFF40);
}