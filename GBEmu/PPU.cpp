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

	BGP = bus.getRegister(0xFF47);

	for (int i = 0x0; i <= 0x9F; i++) { // OAM memory 0xFE00 - 0xFE9F
		OAM[i] = bus.getRegister(0xFE00 + i);
	}

	//for (int i = 0x0; i <= 0x3FFF; i++) {
	//	VRAMPointers[i] = bus.getRegister(0x8000 + i);
	//}

	totalCycles = 0;
	newCycles = 0;

	setState(PPUState::HBlank);

	initSDL();
	tex.reset(SDL_CreateTexture(renderer.get(), SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, 256, 144));
}

void PPU::tick() {
	if (getLCDCFlag(7)) {
		newCycles += bus.getCycles(); //fetch how many cycles have passed
		scanlineCycles += newCycles; // add them to the total number of cycles for the scanline
		totalCycles += newCycles;
		//std::cout << "LCDC: " << +(*LCDC) << std::endl;

		currentState = getState();
		switch (currentState) {
		case PPUState::HBlank:
		{
			if (scanlineCycles >= 456) {
				scanlineCycles -= 456; // new scanline
				if ((*LY) >= 144) {
					setState(PPUState::VBlank);
				}
				else {
					setState(PPUState::OAMScan);
				}
			}
			break;
		}
		case PPUState::VBlank:
		{
			if (totalCycles >= 70224) {
				SDL_UpdateTexture(tex.get(), NULL, framebuffer, 256 * sizeof(uint8_t) * 3);
				SDL_RenderCopy(renderer.get(), tex.get(), NULL, NULL);
				SDL_RenderPresent(renderer.get());
				totalCycles -= 70224; // new frame
				setState(PPUState::OAMScan);
				(*LY) = 0;
			}
			break;
		}
		case PPUState::OAMScan:
		{
			if (scanlineCycles >= 80) {
				for (int i = 0x0; i <= 0x9F; i += 4) {
					Sprite s(*OAM[i], *OAM[i + 1], *OAM[i + 2], *OAM[i + 3]);
					if (s.Xpos > 0 && (*LY + 16) >= s.Ypos && (*LY + 16) < (s.Ypos + 8 + 8 * getLCDCFlag(3)) && spriteBuffer.size() < 10) {
						spriteBuffer.push_back(s);
					}
				}
				//set state to drawing
				setState(PPUState::Drawing);
			}
			break;
		}
		case PPUState::Drawing:
		{
			if (scanlineCycles >= 172) {
				fetchBackground((*LY));
				(*LY)++;
				setState(PPUState::HBlank);
			}
			break;
		}
		}
	}
}

void PPU::initSDL() {
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		std::cout << "FATAL ERROR: could not initalize SDL: " << SDL_GetError() << std::endl;
		throw std::runtime_error("FATAL ERROR: could not initalize SDL!");
	}

	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
		std::cout << "WARNING: Linear texture filtering not enabled!";
	}

	window = std::shared_ptr<SDL_Window>(SDL_CreateWindow("Gameboy Emulator", SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED, 640, 576, SDL_WINDOW_SHOWN),
		[](SDL_Window* window) {
			SDL_DestroyWindow(window);
			window = NULL;
		});

	if (!window) {
		std::cout << "FATAL ERROR: could not create SDL window: " << SDL_GetError() << std::endl;
		throw std::runtime_error("FATAL ERROR: could not create SDL window!");
	}

	renderer = std::shared_ptr<SDL_Renderer>(SDL_CreateRenderer(window.get(), -1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC),
		[](SDL_Renderer* renderer) {
			SDL_DestroyRenderer(renderer);
			renderer = NULL;
		});

	if (!renderer) {
		std::cout << "FATAL ERROR: could not initialize SDL renderer: " << SDL_GetError() << std::endl;
		throw std::runtime_error("FATAL ERROR: could not create SDL renderer!");
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
		*STAT = (*STAT & 0b11111100) + 0b01;
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

void PPU::fetchBackground(uint8_t row) {
	uint16_t tMapBasePointer = 0x9800 + getLCDCFlag(3) * 0x400;
	uint16_t tDataBasePointer = 0x8000 + getLCDCFlag(4) * 0x1000;
	for (int j = 0; j < 256; j++) {
		//wrapping
		uint8_t offsetY = row + (*SCY);
		uint8_t offsetX = j + (*SCX);

		uint8_t tileNumber = bus.readDEBUG(tMapBasePointer + ((offsetY / 8 * 32) + (offsetX / 8)));
		uint8_t colour = 0;
		if (tDataBasePointer == 0x8000) {
			colour = (bus.readDEBUG(tDataBasePointer + (tileNumber * 0x10) + (offsetY % 8 * 2)) >> (7 - (offsetX % 8)) & 0x1) +(bus.readDEBUG(tDataBasePointer + (tileNumber * 0x10) + (offsetY % 8 * 2) + 1) >> (7 - (offsetX % 8)) & 0x1) * 2;
		}
		else {
			colour = (bus.readDEBUG(tDataBasePointer + 0x800 + ((int8_t)tileNumber * 0x10) + (offsetY % 8 * 2)) >> (7 - (offsetX % 8)) & 0x1) + ((bus.readDEBUG(tDataBasePointer + 0x800 + ((int8_t)tileNumber * 0x10) + (offsetY % 8 * 2) + 1) >> (7 - (offsetX % 8)) & 0x1) * 2);
		}
		framebuffer[(row * 256 * 3) + (j * 3)] = colour * 85;
		framebuffer[(row * 256 * 3) + (j * 3) + 1] = colour * 85;
		framebuffer[(row * 256 * 3) + (j * 3) + 2] = colour * 85;
	}
	
}
