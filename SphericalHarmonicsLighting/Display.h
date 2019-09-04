#pragma once
#include <SDL2/SDL.h>

#include "Types.h"

struct Display {
private:
	SDL_Window *  window;
	SDL_GLContext context;

public:
	const u32 width;
	const u32 height;

	Display(u32 width, u32 height, const char* title);
	~Display();

	void update();

	inline void set_title(const char* title) {
		SDL_SetWindowTitle(window, title);
	}
};
