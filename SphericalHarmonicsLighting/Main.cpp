#include <iostream>

#include "Display.h"
#include "Shader.h"

#include "Scene.h"

#include "Types.h"
#include "Util.h"

void GLAPIENTRY glMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n", (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
}

int main(int argc, char** argv) {
	//char title_buffer[1024];
	//strcpy_s(title_buffer, "Spherical Harmonic Lighting - ");

	Display display(1600, 900, "");

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
	//glEnable(GL_CULL_FACE);
	
	printf("OpenGL Info:\n");
	printf("Version:  %s\n",   glGetString(GL_VERSION));
	printf("GLSL:     %s\n",   glGetString(GL_SHADING_LANGUAGE_VERSION));
	printf("Vendor:   %s\n",   glGetString(GL_VENDOR));
	printf("Renderer: %s\n\n", glGetString(GL_RENDERER));

	Shader shader = Shader(DATA_PATH("vertex.glsl"), DATA_PATH("frag.glsl"));
	shader.bind();

	GLuint uni_tbo_texture     = shader.get_uniform("tbo_texture");
	GLuint uni_light_coeffs    = shader.get_uniform("light_coeffs");
	GLuint uni_view_projection = shader.get_uniform("view_projection");

	glUniform1i(uni_tbo_texture, 0);

	Scene scene = Scene();
	scene.init();

	while (!close_window) {
		// Handle SDL events
		while (SDL_PollEvent(&event)) {
			//ImGui_ImplSDL2_ProcessEvent(&event);

			switch (event.type) {
				case SDL_QUIT: close_window = true; break;
			}
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		scene.update(delta_time, SDL_GetKeyboardState(0));
		scene.render(uni_view_projection, uni_light_coeffs);

		display.update();

		now = SDL_GetPerformanceCounter();
		delta_time = (float)(now - last) / (float)SDL_GetPerformanceFrequency();
		last = now;

		printf("%f ms\n", delta_time * 1000.0f);

		//_gcvt_s(title_buffer + 31, 1024, delta_time * 1000, 6);
		//display.set_title(title_buffer);
	}

	return 0;
}
