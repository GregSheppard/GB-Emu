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

	for (int i = 0x0; i <= 0x3FFF; i++) {
		VRAMPointers[i] = bus.getRegister(0x8000 + i);
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
				if (s.Xpos > 0 && (*LY + 16) >= s.Ypos && (*LY+16) < (s.Ypos+8+8*getLCDCFlag(3)) && spriteBuffer.size() < 10) {
					spriteBuffer.push_back(s);
				}
			}
			//set state to drawing
			setState(PPUState::Drawing);
			bus.setCycles(cycles - 80);
		}
		break;
	}
	case PPUState::Drawing:
	{
		for (int xpos = 0; xpos < *WX - 7; xpos++) { //background
			uint8_t backgroundTileNum = fetchBackgroundTileNumber(xpos);
			Tile tile = fetchTile(backgroundTileNum);
		}
		if(getLCDCFlag(5) && )



		
		/*for (int i = 0; i < spriteBuffer.size(); i++) {
			if (spriteBuffer.at(i).Xpos <= xpos) {
				uint8_t tileNum = spriteBuffer.at(i).tileNum; //fetch tile number
				uint8_t sLower = VRAM((0x8000 + tileNum * 16) + 2 * ((*LY + *SCY) % 8)); //fetch lower
				uint8_t sUpper = VRAM((0x8000 + tileNum * 16 + 1) + 2 * ((*LY + *SCY) % 8)); //fetch upper
			}
		}*/

		break;
	}
	}
}

PPUState PPU::getState() {
	int state = (*STAT & 0b00000011);
	switch (state) {
	case 0: return PPUState::HBlank; break;
	case 1: return PPUState::VBlank; break;
	case 2: return PPUState::OAMScan; break;
	case 3: return PPUState::Drawing; break;
	default: std::exit(-1);
	}
}

bool PPU::getLCDCFlag(int bit) {
	return (*LCDC & (1 << bit)) >> bit;
}

void PPU::setLCDCFlag(int bit) {
	*LCDC |= 1 << bit;
}

uint8_t PPU::VRAM(uint16_t addr) {
	return *VRAMPointers[addr - 0x8000];
}

void PPU::setState(PPUState state) {
	switch (state) {
	case PPUState::HBlank:
	{
		*STAT = (*STAT & 0b11111100);
		break;
	}
	case PPUState::VBlank:
	{
		*STAT = (*STAT & 0b11111100) + 0b1;
		break;
	}
	case PPUState::OAMScan:
	{
		*STAT = (*STAT & 0b11111100) + 0b10;
		break;
	}
	case PPUState::Drawing:
	{
		*STAT = (*STAT & 0b11111100) + 0b11;
		break;
	}
	}
}

uint8_t PPU::fetchBackgroundTileNumber(int xpos) {
	uint16_t basePointer = 0x9800;
	uint16_t wraparoundOffset = 32*(((*LY+*SCY) & 0xFF) /8);
	uint16_t xposOffset = xpos + ((*SCX / 8) & 0x1f);
	uint16_t totalOffset = (xposOffset + wraparoundOffset) & 0x3FF;
	if (getLCDCFlag(3)) { //BG tile map select
		uint16_t modeOffset = 0x400; 
		return VRAM(basePointer + modeOffset + totalOffset);
	}
	else {
		return VRAM(basePointer + totalOffset);
	}
}

Tile PPU::fetchTile(uint8_t tileNumber) {
	if (getLCDCFlag(4)) { // 0x8000 method
		uint16_t basePointer = 0x8000;
		uint16_t tileOffset = tileNumber * 16;
		uint16_t lyOffset = 2 * ((*LY + *SCY) % 8);
		uint16_t result = basePointer + tileOffset + lyOffset;
		Tile tile(VRAM(result), VRAM(result + 0x1));

		return tile;
	}
	else { //0x8800 method
		uint16_t basePointer = 0x9000;
		int8_t signedTileOffset = (int8_t)tileNumber * 16;
		uint16_t lyOffset = 2 * ((*LY + *SCY) % 8);
		uint16_t result = basePointer + signedTileOffset + lyOffset;
		Tile tile(VRAM(result), VRAM(result + 0x1));

		return tile;
	}

}