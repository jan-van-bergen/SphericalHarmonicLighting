#include "Shader.h"

#include <fstream>
#include <iostream>

Shader::Shader(const char* vertex_filename, const char* fragment_filename) {
	// Create Program
	program_id = glCreateProgram();

	// Load Shader sources
	vertex_id   = load_shader(vertex_filename,   GL_VERTEX_SHADER);
	fragment_id = load_shader(fragment_filename, GL_FRAGMENT_SHADER);

	// Attach Vertex and Fragment Shaders to the Program
	glAttachShader(program_id, vertex_id);
	glAttachShader(program_id, fragment_id);

	// Link the Program
	glLinkProgram(program_id);

	// Check if linking succeeded
	GLint success;
	glGetProgramiv(program_id, GL_LINK_STATUS, &success);
	if (!success) {
		GLchar info_log[1024];
		glGetProgramInfoLog(program_id, sizeof(info_log), NULL, info_log);
		fprintf(stderr, "Error linking shader program: '%s'\n", info_log);
	}

	// Validate Program
	glValidateProgram(program_id);

	// Check if the Program is valid
	glGetProgramiv(program_id, GL_VALIDATE_STATUS, &success);
	if (!success) {
		GLchar info_log[1024];
		glGetProgramInfoLog(program_id, sizeof(info_log), NULL, info_log);
		fprintf(stderr, "Error validating shader program: '%s'\n", info_log);
	}
}

Shader::~Shader() {
	glDeleteShader(vertex_id);
	glDeleteShader(fragment_id);

	glDeleteProgram(program_id);
}

GLuint Shader::load_shader(const char* filename, GLuint shader_type) const {
	// @SPEED
	std::ifstream t(filename);
	std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());

	const GLchar* src = static_cast<const GLchar*>(str.c_str());

	GLuint shader = glCreateShader(shader_type);

	const GLchar* srcs[1] = { src };
	const GLint   lens[1] = { str.length() };
	glShaderSource(shader, 1, srcs, lens);
	glCompileShader(shader);

	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar info_log[1024];
		glGetShaderInfoLog(shader, sizeof(info_log), NULL, info_log);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", shader_type, info_log);
	}

	return shader;
}
