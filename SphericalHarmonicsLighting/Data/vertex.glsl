#version 410

// Attributes
layout (location = 0) in vec3 position;

// Varyings
layout (location = 0) out vec3 frag_colour;

const int SH_COEFFICIENT_COUNT = 25;
uniform samplerBuffer tbo_texture;

uniform mat4 view_projection;

void main() {
	float r = texelFetch(tbo_texture, gl_VertexID * SH_COEFFICIENT_COUNT + 0).r;
	float g = texelFetch(tbo_texture, gl_VertexID * SH_COEFFICIENT_COUNT + 1).r;
	float b = texelFetch(tbo_texture, gl_VertexID * SH_COEFFICIENT_COUNT + 2).r;

	frag_colour = vec3(r, g, b);
	
	gl_Position = view_projection * vec4(position, 1.0f);
}
