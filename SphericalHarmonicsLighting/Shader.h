#pragma once
#include <string>

#include <GL/glew.h>

class Shader
{
public:
	Shader(const char* vertex_filename, const char* fragment_filename);
	~Shader();

	inline void bind() const {
		glUseProgram(program_id);
	}

	inline GLuint get_uniform(const char* name) const {
		return glGetUniformLocation(program_id, name);
	}

	inline void unbind() {
		glUseProgram(0);
	}

private:
	GLuint load_shader(const char* filename, GLuint shader_type) const;

	GLuint program_id;
	GLuint vertex_id;
	GLuint fragment_id;
};
