#version 410

// Varyings
layout (location = 0) in vec3 frag_colour;

// Output
layout (location = 0) out vec4 colour;

void main() {
	colour = vec4(frag_colour, 1.0f);
}
