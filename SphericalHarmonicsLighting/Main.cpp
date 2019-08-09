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
	//glEnable(GL_CULL_FACE);
	
	printf("OpenGL Info:\n");
	printf("Version:  %s\n",   glGetString(GL_VERSION));
	printf("GLSL:     %s\n",   glGetString(GL_SHADING_LANGUAGE_VERSION));
	printf("Vendor:   %s\n",   glGetString(GL_VENDOR));
	printf("Renderer: %s\n\n", glGetString(GL_RENDERER));

	Shader shader = Shader(DATA_PATH("vertex.glsl"), DATA_PATH("frag.glsl"));
	shader.bind();

	GLuint uni_tbo_texture     = shader.get_uniform("tbo_texture");
	GLuint uni_view_projection = shader.get_uniform("view_projection");

	Scene scene = Scene();
	scene.init();

	const glm::vec2 vertices[3] = {
		glm::vec2( 1.0f, -1.0f),
		glm::vec2(-1.0f, -1.0f),
		glm::vec2( 0.0f,  1.0f)
	};

	const u8 indices[3] = { 0, 1, 2 };

	const float coeffs[9] = {
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 
		0.0f, 0.0f, 1.0f
	};

	GLuint vbo, ibo, tbo, tbo_tex;

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glGenBuffers(1, &tbo);
	glBindBuffer(GL_TEXTURE_BUFFER, tbo);
	glBufferData(GL_TEXTURE_BUFFER, sizeof(coeffs), coeffs, GL_STATIC_DRAW);
	glGenTextures(1, &tbo_tex);


	while (!close_window) {
		// Handle SDL events
		while (SDL_PollEvent(&event)) {
			//ImGui_ImplSDL2_ProcessEvent(&event);

			switch (event.type) {
				case SDL_QUIT: close_window = true; break;
			}
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//scene.render(uni_tbo_texture, uni_view_projection);


		glUniformMatrix4fv(uni_view_projection, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));


		// Bind TBO
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_BUFFER, tbo_tex);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, tbo); // @TODO: check if this is required for every frame?
		glUniform1i(uni_tbo_texture, 0);


		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_BYTE, 0);

		glDisableVertexAttribArray(0);


		display.update();

		now = SDL_GetPerformanceCounter();
		delta_time = (float)(now - last) / (float)SDL_GetPerformanceFrequency();
		last = now;
	}

	return 0;
}
