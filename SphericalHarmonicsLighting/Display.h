#pragma once
#include <SDL2/SDL.h>

#include "Types.h"

class Display {
public:
	Display(u32 width, u32 height, const char* title);
	~Display();

	void update();

	inline void set_title(const char* title) {
		SDL_SetWindowTitle(window, title);
	}

	inline u32 get_width()  { return width; }
	inline u32 get_height() { return height; }

private:
	const u32 width;
	const u32 height;

	SDL_Window *  window;
	SDL_GLContext context;
};
