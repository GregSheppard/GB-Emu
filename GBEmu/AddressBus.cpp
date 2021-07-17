#include "AddressBus.h"
#include <iostream>

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

void AddressBus::init() {
	memset(memory, 0, sizeof(memory));
}

void AddressBus::write(uint16_t address, uint8_t value) {
	if (address >= 0x0000 && address <= 0x7FFF) { //ROM BANKS
		//std::cout << "Attempted write to ROM bank area." << std::endl;
		memory[address] = value;
	}
	else if (address >= 0x8000 && address <= 0xDFFF) { //VRAM and RAMS
		memory[address] = value;
	}
	else if (address >= 0xE000 && address <= 0xFDFF) { //ECHO RAM
		memory[address] = value;
	}
	else if (address >= 0xFE00 && address <= 0xFE9F) { //SPRITE ATTRIBUTE TABLE
		memory[address] = value;
	}
	else if (address >= 0xFEA0 && address <= 0xFEFF) { // FORBIDDEN AREA
		std::cout << "Attempted write to unusable area. forbidden." << std::endl;
		std::exit(-1);
	}
	else if ((address >= 0xFF00 && address <= 0xFF7F) && address != 0xFF01) { // IO REGISTERS
		memory[address] = value;
	}
	else if (address == 0xFF01) {
		std::cout << (char)value; //print SB writes
		memory[address] = value;
	}
	else if (address >= 0xFF80 && address <= 0xFFFE) { //HIGH RAM
		memory[address] = value;
	}
	else if (address == 0xFFFF) { //interrupt enable register
		memory[address] = value;
	}

}

uint8_t AddressBus::read(uint16_t address) {
	return memory[address];
}