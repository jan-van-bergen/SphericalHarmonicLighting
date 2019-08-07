#include <iostream>

#include "Display.h"

void GLAPIENTRY glMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n", (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
}

int main(int argc, char** argv) {
	Display display(1600, 900, "Spherical Harmonic Lighting");

	Uint64 now = SDL_GetPerformanceCounter();
	Uint64 last = 0;
	float delta_time = 0;

	SDL_Event event;
	bool close_window = false;

#if _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(glMessageCallback, NULL);
#endif

	// OpenGL settings
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_CLIP_DISTANCE0);

	printf("OpenGL Info:\n");
	printf("Version:  %s\n",   glGetString(GL_VERSION));
	printf("GLSL:     %s\n",   glGetString(GL_SHADING_LANGUAGE_VERSION));
	printf("Vendor:   %s\n",   glGetString(GL_VENDOR));
	printf("Renderer: %s\n\n", glGetString(GL_RENDERER));

	while (!close_window) {
		// Handle SDL events
		while (SDL_PollEvent(&event)) {
			//ImGui_ImplSDL2_ProcessEvent(&event);

			switch (event.type) {
				case SDL_QUIT: close_window = true; break;
			}
		}

		//display.begin_gui();
		display.update();

		now = SDL_GetPerformanceCounter();
		delta_time = (float)(now - last) / (float)SDL_GetPerformanceFrequency();
		last = now;
	}

	return 0;
}
