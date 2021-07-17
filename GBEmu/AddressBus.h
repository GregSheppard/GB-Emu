#pragma once
#include <cstdint>

/*
MEMORY MAP:
0x0000 ~ 0x3FFF - ROM BANK 0
0x4000 ~ 0x7FFF - ROM BANK 01-N
0x8000 ~ 0x9FFF - VRAM
0xA000 ~ 0xBFFF - EXTERNAL RAM - SWITCHABLE
0xC000 ~ 0xCFFF - WRAM BANK 0
0xD000 ~ 0xDFFF - WRAM BANK 1-N
0xE000 ~ 0xFDFF - ECHO RAM
0xFE00 ~ 0xFE9F - SPRITE ATTRIBUTE TABLE
0xFEA0 ~ 0xFEFF - UNUSABLE
0xFF00 ~ 0xFF7F - I/O REGISTERS
0xFF80 - 0xFFFE - HIGH RAM

NOTABLE
0xFFFF - INTERRUPTS REGISTER
0xFF01 - SERIAL BUS
*/

class AddressBus {
private:
	uint8_t memory[0xFFFF + 1];
public:
	void init();
	void write(uint16_t address, uint8_t value);
	uint8_t read(uint16_t address);
};
