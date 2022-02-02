#include <SDL.h>
#include <memory>
#include <iostream>

class UI {
private:
	const int WINDOW_WIDTH = 640;
	const int WINDOW_HEIGHT = 576;
	std::shared_ptr<SDL_Renderer> renderer;
	std::shared_ptr<SDL_Window> window = NULL;

	//emulator
	SDL_Rect eBackground[32][32];
	SDL_Rect eWindow[32][32];

public:
	UI();
	void draw();

};