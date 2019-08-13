#version 410

// Attributes
layout (location = 0) in vec3 position;

// Varyings
layout (location = 0) out vec3 frag_colour;

const int SH_COEFFICIENT_COUNT = 25;
uniform samplerBuffer tbo_texture;

uniform vec3 light_coeffs[SH_COEFFICIENT_COUNT];

uniform mat4 view_projection;

//uniform vec3 diffuse_colour;

void main() {
	vec3 colour = vec3(0.0f, 0.0f, 0.0f);

	for (int i = 0; i < SH_COEFFICIENT_COUNT; i++) {
		colour += light_coeffs[i] * texelFetch(tbo_texture, gl_VertexID * SH_COEFFICIENT_COUNT + i).rgb;
	}

	frag_colour = colour;
	
	gl_Position = view_projection * vec4(position, 1.0f);
}
