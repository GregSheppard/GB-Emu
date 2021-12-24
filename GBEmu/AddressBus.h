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
	//ROM
	uint8_t* ROM;
	unsigned int ROMSize;
	void loadROMFromFile();
	void insertCartridge();

	//memory
	uint8_t bank0[0x3FFF + 1];
	uint8_t bankN[0x3FFF + 1];
	uint8_t VRAM[0x3FFF + 1];
	uint8_t ERAM[0x3FFF + 1];
	uint8_t WRAM[0x7FFF + 1];
	uint8_t OAM[0x9F + 1];
	uint8_t IO[0x7F + 1];
	uint8_t HRAM[0x7E + 1];
	uint8_t interruptEnable;

	long cycles;

public:
	AddressBus();
	void write(uint16_t address, uint8_t value);
	uint8_t read(uint16_t address);
	void addCycles(short cycles);
	uint8_t getInterruptFlag();
	uint8_t getInterruptEnable();
	uint8_t* getRegister(uint16_t addr);
};
