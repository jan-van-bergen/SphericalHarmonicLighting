#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <SDL2/SDL.h>

#include "Types.h"

class Display {
public:
	Display(u32 width, u32 height, const char* title);
	~Display();

	void update();

	void begin_gui();

	inline void set_title(const char* title) {
		SDL_SetWindowTitle(window, title);
	}

	inline u32 get_width()  { return width; }
	inline u32 get_height() { return height; }

	inline bool is_closed() { return closed; }

private:
	const u32 width;
	const u32 height;

	SDL_Window* window;
	SDL_GLContext context;

	bool closed;
};
