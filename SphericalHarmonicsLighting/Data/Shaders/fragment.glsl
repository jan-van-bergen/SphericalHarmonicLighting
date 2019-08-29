#version 410

// Varyings
layout (location = 0) in vec3 colour_in;

// Output
layout (location = 0) out vec4 colour_out;

void main() {
	colour_out = vec4(colour_in, 1.0f);
}
