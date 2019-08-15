#version 410

layout (location = 0) in vec3 position_in;

uniform mat4 view_projection;

void main() {
	gl_Position = view_projection * vec4(position_in, 1.0f);
}