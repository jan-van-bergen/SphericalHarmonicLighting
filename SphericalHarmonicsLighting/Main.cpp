#include <iostream>

#include "Display.h"
#include "Shader.h"

#include "Scene.h"

#include "Types.h"
#include "Util.h"

#include "StringHelper.h"

// Forces NVIDIA driver to be used 
extern "C" { _declspec(dllexport) unsigned NvOptimusEnablement = 0x00000001; }

void GLAPIENTRY glMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n", (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);

	if (type == GL_DEBUG_TYPE_ERROR) {
		abort();
	}
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
	glEnable(GL_MULTISAMPLE);
	
	printf("OpenGL Info:\n");
	printf("Version:  %s\n",   glGetString(GL_VERSION));
	printf("GLSL:     %s\n",   glGetString(GL_SHADING_LANGUAGE_VERSION));
	printf("Vendor:   %s\n",   glGetString(GL_VENDOR));
	printf("Renderer: %s\n\n", glGetString(GL_RENDERER));

	Shader debug_shader = Shader(DATA_PATH("Shaders/debug_vertex.glsl"), DATA_PATH("Shaders/debug_fragment.glsl"));
	GLuint uni_debug_view_projection = debug_shader.get_uniform("view_projection");

	Scene scene = Scene();
	scene.init();

	bool print_frame_time = false;

	while (!close_window) {
		// Handle SDL events
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_KEYUP: if (event.key.keysym.sym == SDLK_e) print_frame_time = !print_frame_time; break;

				case SDL_QUIT: close_window = true; break;
			}
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		scene.update(delta_time, SDL_GetKeyboardState(0));

		scene.render();

		debug_shader.bind();
				
		// Uncomment to see a wireframe rendering of the all the BVH in the Scene:
		//scene.debug(uni_debug_view_projection);

		display.update();

		now = SDL_GetPerformanceCounter();
		delta_time = (float)(now - last) / (float)SDL_GetPerformanceFrequency();
		last = now;

		if (print_frame_time) {
			printf("%f ms\n", delta_time * 1000.0f);
		}
	}

	return 0;
}
