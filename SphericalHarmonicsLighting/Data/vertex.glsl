#version 410

// Attributes
layout (location = 0) in vec3 position;

// Varyings
layout (location = 0) out vec3 frag_colour;

const int SH_COEFFICIENT_COUNT = 25;
uniform samplerBuffer tbo_texture;

uniform float light_coeffs[SH_COEFFICIENT_COUNT];

uniform mat4 view_projection;

//uniform vec3 diffuse_colour;

void main() {
	float brightness = 0.0f;

	for (int i = 0; i < SH_COEFFICIENT_COUNT; i++) {
		brightness += light_coeffs[i] * texelFetch(tbo_texture, gl_VertexID * SH_COEFFICIENT_COUNT + i).r;
	}

	frag_colour = vec3(brightness);// * diffuse_colour;
	
	gl_Position = view_projection * vec4(position, 1.0f);
}
