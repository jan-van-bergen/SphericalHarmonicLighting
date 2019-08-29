#version 410

// Attributes
layout (location = 0) in vec3 position_in;

// Varyings
layout (location = 0) out vec3 colour_out;

uniform samplerBuffer tbo_texture;

uniform vec3 light_coeffs[SH_COEFFICIENT_COUNT];

uniform mat4 view_projection;

//uniform vec3 diffuse_colour;

void main() {
	vec3 colour = vec3(0.0f, 0.0f, 0.0f);

	// Compute the spherical integral between the lighting function and the transfer function.
	// Using Spherical Harmonics this means a simple dot product between two SH coefficient vectors.
	for (int i = 0; i < SH_COEFFICIENT_COUNT; i++) {
		colour += light_coeffs[i] * texelFetch(tbo_texture, gl_VertexID * SH_COEFFICIENT_COUNT + i).rgb;
	}

	colour_out = colour;
	
	gl_Position = view_projection * vec4(position_in, 1.0f);
}
