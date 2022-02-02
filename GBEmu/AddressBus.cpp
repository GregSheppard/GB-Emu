#include "AddressBus.h"
#include <iostream>
#include <fstream>

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

AddressBus::AddressBus() {
	loadROMFromFile();
	//loadBootrom();
	insertCartridge();
	cycles = 0;
}

void AddressBus::loadROMFromFile() {
	std::string romName;
	std::cout << "enter ROM: ";
	std::cin >> romName;
	std::ifstream file(romName, std::ios::in | std::ios::binary | std::ios::ate);
	std::streampos size;
	char* memblock;
	if (file.is_open()) {
		std::cout << "opened ROM." << std::endl;
		size = file.tellg();
		ROMSize = size;
		memblock = new char[size];
		file.seekg(0, std::ios::beg);
		file.read(memblock, size);
		file.close();

		ROM = new uint8_t[size];
		for (int i = 0; i < size; i++) {
			ROM[i] = memblock[i];
		}
	}
	else {
		std::cout << "Could not open ROM!" << std::endl;
		std::exit(-1);
	}
}

void AddressBus::loadBootrom() {
	for (int i = 0; i <= 0x100; i++) {
		bank0[i] = ROM[i];
	}
}

void AddressBus::insertCartridge() {
	for (int i = 0; i <= 0x3FFF; i++) {
		bank0[i] = ROM[i];
	}
	for (int i = 0x4000; i <= 0x7FFF; i++) { // TODO: add support for mbcs
		bankN[i-0x4000] = ROM[i];
	}
}

void AddressBus::write(uint16_t addr, uint8_t value) {
	cycles += 4;
	//std::cout << "write to bus at address " << std::hex << addr << " with value " << +value << std::endl;
	if (addr >= 0x0 && addr <= 0x3FFF) {
		std::cout << "[WARNING] Attempted to write to ROM" << std::endl;
	}
	else if (addr >= 0x4000 && addr <= 0x7FFF) {
		std::cout << "[WARNING] Attempted to write to ROM" << std::endl;
	}
	else if (addr >= 0x8000 && addr <= 0x9FFF) {
		//std::cout << "write to VRAM at address " << addr << " value: " << +value << std::endl;
		VRAM[addr - 0x8000] = value;
	}
	else if (addr >= 0xA000 && addr <= 0xBFFF) {
		ERAM[addr - 0xA000] = value;
	}
	else if (addr >= 0xC000 && addr <= 0xDFFF) {
		WRAM[addr - 0xC000] = value;
	}
	else if (addr >= 0xE000 && addr <= 0xFDFF) { //echo ram
		WRAM[addr - 0xE000] = value;
	}
	else if (addr >= 0xFE00 && addr <= 0xFE9F) {
		OAM[addr - 0xFE00] = value;
	}
	else if (addr >= 0xFF00 && addr <= 0xFF7F) {
		IO[addr - 0xFF00] = value;
		if (addr == 0xFF01) {
			std::cout << (char)value;
		}
		if (addr == 0xFF04) {
			IO[addr - 0xFF00] = 0; //hacky fix later
		}
	}
	else if (addr >= 0xFF80 && addr <= 0xFFFE) {
		HRAM[addr - 0xFF80] = value;
	}
	else if (addr == 0xFFFF) {
		interruptEnable = value;
	}
}

void AddressBus::writeDEBUG(uint16_t addr, uint8_t value) {
	//std::cout << "write to bus at address " << std::hex << addr << " with value " << +value << std::endl;
	if (addr >= 0x0 && addr <= 0x3FFF) {
		std::cout << "[WARNING] Attempted to write to ROM" << std::endl;
	}
	else if (addr >= 0x4000 && addr <= 0x7FFF) {
		std::cout << "[WARNING] Attempted to write to ROM" << std::endl;
	}
	else if (addr >= 0x8000 && addr <= 0x9FFF) {
		//std::cout << "write to VRAM at address " << addr << " value: " << value << std::endl;
		VRAM[addr - 0x8000] = value;
	}
	else if (addr >= 0xA000 && addr <= 0xBFFF) {
		ERAM[addr - 0xA000] = value;
	}
	else if (addr >= 0xC000 && addr <= 0xDFFF) {
		WRAM[addr - 0xC000] = value;
	}
	else if (addr >= 0xE000 && addr <= 0xFDFF) { //echo ram
		WRAM[addr - 0xE000] = value;
	}
	else if (addr >= 0xFE00 && addr <= 0xFE9F) {
		OAM[addr - 0xFE00] = value;
	}
	else if (addr >= 0xFF00 && addr <= 0xFF7F) {
		IO[addr - 0xFF00] = value;
		if (addr == 0xFF01) {
			std::cout << (char)value;
		}
		if (addr == 0xFF04) {
			IO[addr - 0xFF00] = 0; //hacky fix later
		}
	}
	else if (addr >= 0xFF80 && addr <= 0xFFFE) {
		HRAM[addr - 0xFF80] = value;
	}
	else if (addr == 0xFFFF) {
		interruptEnable = value;
	}
}

uint8_t AddressBus::read(uint16_t addr) {
	cycles += 4;
	//std::cout << "read from bus at address " << addr << std::endl;
	if (addr >= 0x0 && addr <= 0x3FFF) {
		return bank0[addr];
	}
	else if (addr >= 0x4000 && addr <= 0x7FFF) {
		return bankN[addr - 0x4000];
	}
	else if (addr >= 0x8000 && addr <= 0x9FFF) {
		return VRAM[addr - 0x8000];
	}
	else if (addr >= 0xA000 && addr <= 0xBFFF) {
		return ERAM[addr - 0xA000];
	}
	else if (addr >= 0xC000 && addr <= 0xDFFF) {
		return WRAM[addr - 0xC000];
	}
	else if (addr >= 0xE000 && addr <= 0xFDFF) { //echo ram
		return WRAM[addr - 0xE000];
	}
	else if (addr >= 0xFE00 && addr <= 0xFE9F) {
		return OAM[addr - 0xFE00];
	}
	else if (addr >= 0xFF00 && addr <= 0xFF7F) {
		if (addr == 0xFF00) {
			std::cout << "check joypad" << std::endl;
			return 0xFF;
		}
		else {
			return IO[addr - 0xFF00];
		}
	}
	else if (addr >= 0xFF80 && addr <= 0xFFFE) {
		return HRAM[addr - 0xFF80];
	}
	else if (addr == 0xFFFF) {
		return interruptEnable;
	}
}

uint8_t AddressBus::readDEBUG(uint16_t addr) {
	//std::cout << "read from bus at address 0x" << std::hex << +addr << std::endl;
	if (addr >= 0x0 && addr <= 0x3FFF) {
		return bank0[addr];
	}
	else if (addr >= 0x4000 && addr <= 0x7FFF) {
		return bankN[addr - 0x4000];
	}
	else if (addr >= 0x8000 && addr <= 0x9FFF) {
		return VRAM[addr - 0x8000];
	}
	else if (addr >= 0xA000 && addr <= 0xBFFF) {
		return ERAM[addr - 0xA000];
	}
	else if (addr >= 0xC000 && addr <= 0xDFFF) {
		return WRAM[addr - 0xC000];
	}
	else if (addr >= 0xE000 && addr <= 0xFDFF) { //echo ram
		return WRAM[addr - 0xE000];
	}
	else if (addr >= 0xFE00 && addr <= 0xFE9F) {
		return OAM[addr - 0xFE00];
	}
	else if (addr >= 0xFF00 && addr <= 0xFF7F) {
		if (addr == 0xFF00) {
			std::cout << "check joypad" << std::endl;
			return 0xFF;
		}
		else {
			return IO[addr - 0xFF00];
		}
	}
	else if (addr >= 0xFF80 && addr <= 0xFFFE) {
		return HRAM[addr - 0xFF80];
	}
	else if (addr == 0xFFFF) {
		return interruptEnable;
	}
}

uint8_t* AddressBus::getRegister(uint16_t addr) {
	if (addr >= 0x0 && addr <= 0x3FFF) {
		return &bank0[addr];
	}
	else if (addr >= 0x4000 && addr <= 0x7FFF) {
		return &bankN[addr - 0x4000];
	}
	else if (addr >= 0x8000 && addr <= 0x9FFF) {
		return &VRAM[addr - 0x8000];
	}
	else if (addr >= 0xA000 && addr <= 0xBFFF) {
		return &ERAM[addr - 0xA000];
	}
	else if (addr >= 0xC000 && addr <= 0xDFFF) {
		return &WRAM[addr - 0xC000];
	}
	else if (addr >= 0xE000 && addr <= 0xFDFF) { //echo ram
		return &WRAM[addr - 0xE000];
	}
	else if (addr >= 0xFE00 && addr <= 0xFE9F) {
		return &OAM[addr - 0xFE00];
	}
	else if (addr >= 0xFF00 && addr <= 0xFF7F) {
		return &IO[addr - 0xFF00];
	}
	else if (addr >= 0xFF80 && addr <= 0xFFFE) {
		return &HRAM[addr - 0xFF80];
	}
	else if (addr == 0xFFFF) {
		return &interruptEnable;
	}
}

void AddressBus::addCycles(int cycle) {
	cycles += cycle;
}

void AddressBus::setCycles(int newCycles) {
	cycles = newCycles;
}

int AddressBus::getCycles() {
	return cycles;
}

uint8_t AddressBus::getInterruptEnable() {
	return interruptEnable;
}

uint8_t AddressBus::getInterruptFlag() {
	return IO[0xF];
}