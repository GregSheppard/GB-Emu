#include "GraphicsTest.h"

struct TileData {
	short data[8][8];
};

GraphicsTest::GraphicsTest(AddressBus& _bus) : bus(_bus) {
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

	initSDL();
}

void GraphicsTest::tick() {
	for (int i = 0; i < 3; i++) {
		std::cout << std::hex << +VRAM(0x8000 + i) << std::endl;
	}
	
	
	/*std::vector<TileData> tiles;
	for (int tileNum = 0; tileNum < 383; tileNum++) {
		TileData d;
		for (int y = 0; y < 8; y++) {
			uint8_t l = VRAM(0x8000 + tileNum*0x10 + 2*y);
			uint8_t u = VRAM(0x8000 + tileNum * 0x10 + 0x1 + 2*y);
			std::cout << "lower: " << +l << " upper: " << +u << std::endl;
			for (int pixel = 0; pixel < 8; pixel++) {
				d.data[y][pixel] = ((l & (0x1 << pixel)) >> pixel) + ((u & (0x1 << pixel)) >> pixel - 1);
			}
		}
		tiles.push_back(d);

	}

	for (int t = 0; t < tiles.size(); t++) {
		for (int y = 0; y < 8; y++) {
			for (int x = 0; x < 8; x++) {
				int colour = tiles.at(t).data[y][x] * 85;
				//std::cout << "Tile: " << t << " x: " << x << " y: " << y << " color: " << colour << std::endl;
				SDL_SetRenderDrawColor(renderer.get(), colour, colour, colour, 255);
				SDL_RenderDrawPoint(renderer.get(), x, y);
				SDL_RenderPresent(renderer.get());
			}
		}
	}
	*/
}

uint8_t GraphicsTest::VRAM(uint16_t addr) {
	return *VRAMPointers[addr - 0x8000];
}

void GraphicsTest::initSDL() {
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