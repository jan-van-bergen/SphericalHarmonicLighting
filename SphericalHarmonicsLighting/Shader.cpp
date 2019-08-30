#include "Shader.h"
#include <fstream>
#include <iostream>

#include "Types.h"
#include "Util.h"

#include "StringHelper.h"
#include "ChunkBuffer.h"

#include "ScopedTimer.h"

Shader::Shader(const char* vertex_filename, const char* fragment_filename, const char* geometry_filename, const Defines& defines) {
	// Create Program
	program_id = glCreateProgram();

	// Load Shader Vertex and Fragment sources
	vertex_id   = load_shader(vertex_filename,   GL_VERTEX_SHADER,   defines);
	fragment_id = load_shader(fragment_filename, GL_FRAGMENT_SHADER, defines);

	// Attach Vertex and Fragment Shaders to the Program
	glAttachShader(program_id, vertex_id);
	glAttachShader(program_id, fragment_id);

	// If a geometry shader is speciefied, load its source and attach it to the Program
	if (geometry_filename) {
		geometry_id = load_shader(geometry_filename, GL_GEOMETRY_SHADER, defines);

		glAttachShader(program_id, geometry_id);
	} else {
		geometry_id = INVALID; // Don't use Geometry Shader
	}

	// Link the Program
	glLinkProgram(program_id);

	// Check if linking succeeded
	GLint success;
	glGetProgramiv(program_id, GL_LINK_STATUS, &success);
	if (!success) {
		GLchar info_log[1024];
		glGetProgramInfoLog(program_id, sizeof(info_log), NULL, info_log);
		fprintf(stderr, "Error linking shader program: '%s'\n", info_log);

		abort();
	}

	// Validate Program
	glValidateProgram(program_id);

	// Check if the Program is valid
	glGetProgramiv(program_id, GL_VALIDATE_STATUS, &success);
	if (!success) {
		GLchar info_log[1024];
		glGetProgramInfoLog(program_id, sizeof(info_log), NULL, info_log);
		fprintf(stderr, "Error validating shader program: '%s'\n", info_log);

		abort();
	}
}

Shader::~Shader() {
	glDeleteShader(vertex_id);
	glDeleteShader(fragment_id);

	if (geometry_id != INVALID) {
		glDeleteShader(geometry_id);
	}

	glDeleteProgram(program_id);
}

void replace_defines_and_add(ChunkBuffer<char>& source, int length, const char * src, const Shader::Defines& defines) {
	// Index that indicates where we should currently start copying
	int start = 0;

	int copy_length = length;

	if (defines.count > 0 ) {
		// Iterate over the chars in src
		for (int i = 0; i < length; i++) {
			// Iterate over the define names
			for (int j = 0; j < defines.count; j++) {
				int k = 0;

				// While src and the current define name match
				while (src[i + k] == defines.names[j][k]) {
					k++;

					if (i + k >= length) break;

					// If the match held until the end of the define name
					if (defines.names[j][k] == NULL) {
						// Copy up until where the current define name starts
						source.add(i - start, src + start);

						// Copy the define definition in stead of the name
						int definition_length = strlen(defines.definitions[j]);
						source.add(definition_length, defines.definitions[j]);

						// Mark the fact that we have copied up to i + k,
						// so that next time we copy we start at this index
						start = i + k;

						// Update i and exit the loop, we don't want multiple defines to match
						i += k - 1;
						goto Exit_Definition_Loop;
					}
				}
			}
		Exit_Definition_Loop:;
		}
	}

	// Copy the remaining part of src
	source.add(length - start, src + start);
}

// Recursively loads shader source file using the #include directive
void load_shader_source(const char * path, const char * filename, ChunkBuffer<char>& source, const Shader::Defines& defines) {
	// Open the Shader file
	FILE * f;
	if (fopen_s(&f, filename, "rb") != 0) {
		abort(); // File could not be opened!
	}

	// Find the length of the file in bytes
	fseek(f, 0, SEEK_END);
	long file_size = ftell(f);
	fseek(f, 0, SEEK_SET);

	// Copy the file contents into a buffer
	char *str = new char[file_size + 1];
	fread(str, file_size, 1, f);
	fclose(f);

	// Make sure the string is null terminated
	str[file_size] = NULL;

	// Include keyword and its length
	const char * include        = "#include";
	const int    include_length = strlen(include);

	const int path_length = strlen(path);

	char include_filename_full[1024];
	memcpy(include_filename_full, path, path_length);

	int start = 0;

	while (true) {
		// Try to find the string "#include" in the source str, break if no more occurences
		int pos = StringHelper::first_index_of(include, str, start);
		if (pos == -1) break;

		// Copy from start to pos
		replace_defines_and_add(source, pos - start, str + start, defines);

		// Skip the include itself
		pos += include_length;

		// There can be zero or more white spaces after #include
		while (str[pos] == ' ') pos++;

		if (str[pos] == '"') {
			// Find the name of the file we are trying to include by iterating until we encounter a closing "
			int end = pos + 1;
			while (str[end] != '"') end++;

			// Copy the filename between the quotation marks
			int include_filename_length = end - pos - 1;
			assert(path_length + include_filename_length < 1024); // Assert that it fits in the char buffer

			StringHelper::substr(include_filename_full + path_length, str, pos + 1, include_filename_length);

			// Recurse in case the included file includes another file
			load_shader_source(path, include_filename_full, source, defines);

			// Continue after the closing " of the include
			start = end + 1;
		} else {
			abort(); // only " is allowed for includes
		}
	}

	// Copy from start to end of str
	replace_defines_and_add(source, strlen(str) - start, str + start, defines);

	delete[] str;
}

GLuint Shader::load_shader(const char* filename, GLuint shader_type, const Defines& defines) const {
	ScopedTimer timer(filename);

	// Buffer in which source file will be accumulated
	ChunkBuffer<char> source;

	// Extract the Path from the filename
	int filename_length       = strlen(filename);
	int filename_without_path = filename_length - 1;

	while (filename[filename_without_path] != '/') filename_without_path--;

	char path[1024];
	StringHelper::substr(path, filename, 0, filename_without_path + 1);

	// Load shader source recursively
	load_shader_source(path, filename, source, defines);

	// Copy from the ChunkBuffer into a contiguous buffer
	int source_length = source.size();
	char * data = new char[source_length];
	source.copy_out(data, 0, source_length);

	const GLchar* srcs[1] = { data };
	const GLint   lens[1] = { source_length };

	// Create the Shader
	GLuint shader = glCreateShader(shader_type);
	glShaderSource(shader, 1, srcs, lens);
	glCompileShader(shader);

	delete[] data;

	// Check if the Shader was succesfully created
	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar info_log[1024];
		glGetShaderInfoLog(shader, sizeof(info_log), NULL, info_log);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", shader_type, info_log);
	}

	return shader;
}
