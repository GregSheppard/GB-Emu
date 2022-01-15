#include "PPU.h"

PPU::PPU(AddressBus& _bus) : bus(_bus) {
	WY = bus.getRegister(0xFF4A);
	WX = bus.getRegister(0xFF4B);

	SCY = bus.getRegister(0xFF42);
	SCX = bus.getRegister(0xFF43);

	LY = bus.getRegister(0xFF44);
	LYC = bus.getRegister(0xFF45);

	LCDC = bus.getRegister(0xFF40);
	STAT = bus.getRegister(0xFF41);

	for (int i = 0x0; i <= 0x9F; i++) { // OAM memory 0xFE00 - 0xFE9F
		OAM[i] = bus.getRegister(0xFE00 + i);
	}
}

void PPU::tick() {
	int cycles = bus.getCycles();
	currentState = getState();
	switch (currentState) {
	case PPUState::HBlank:
	{
		break;
	}
	case PPUState::VBlank:
	{
		break;
	}
	case PPUState::OAMScan:
	{
		if (cycles >= 80) {
			for (int i = 0x0; i <= 0x9F; i+=4) {
				Sprite s(*OAM[i], *OAM[i+1], *OAM[i+2], *OAM[i+3]);
				if (s.Xpos > 0 && (*LY + 16) >= s.Ypos && (*LY+16) < (s.Ypos+8+8*((*LCDC & 0x00000100) >> 2)) && spriteBuffer.size() < 10) {
					spriteBuffer.push_back(s);
				}
			}
			bus.setCycles(cycles - 80);
		}
		break;
	}
	case PPUState::Drawing:
	{
		break;
	}
	}
}

PPUState PPU::getState() {
	int state = (*LCDC & 0x00000011);
	switch (state) {
	case 0: return PPUState::HBlank; break;
	case 1: return PPUState::VBlank; break;
	case 2: return PPUState::OAMScan; break;
	case 3: return PPUState::Drawing; break;
	default: std::exit(-1);
	}
}