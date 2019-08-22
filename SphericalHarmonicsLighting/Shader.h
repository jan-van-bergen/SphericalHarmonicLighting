#pragma once
#include <string>

#include <GL/glew.h>

class Shader {
public:
	struct Defines {
		const int count;
		const char ** names;
		const char ** definitions;

		inline Defines(int count, const char ** names, const char ** definitions) : 
			count(count), 
			names(names), 
			definitions(definitions) 
		{ }
	};

	Shader(const char* vertex_filename, const char* fragment_filename, const char* geometry_filename = nullptr, const Defines& defines = Defines(0, nullptr, nullptr));
	~Shader();

	inline void bind() const {
		glUseProgram(program_id);
	}

	inline GLuint get_uniform(const char* name) const {
		return glGetUniformLocation(program_id, name);
	}

	static void unbind() {
		glUseProgram(0);
	}

private:
	GLuint load_shader(const char* filename, GLuint shader_type, const Defines& defines) const;

	GLuint program_id;
	GLuint vertex_id;
	GLuint fragment_id;
	GLuint geometry_id;
};
