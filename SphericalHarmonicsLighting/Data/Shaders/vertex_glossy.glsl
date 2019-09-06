#version 410
#include "sh.h"

// Attributes
layout (location = 0) in vec3 position_in;
layout (location = 1) in vec3 normal_in;

// Varyings
layout (location = 0) out vec3 colour_out;

uniform samplerBuffer tbo_texture;

uniform vec3 light_coeffs[SH_COEFFICIENT_COUNT];
uniform vec3 brdf_coeffs [SH_NUM_BANDS];

uniform vec3 camera_position;
uniform mat4 view_projection;

//uniform vec3 diffuse_colour;

void main() {
	vec3 transfer_coeffs[SH_COEFFICIENT_COUNT];
	// Initialize transfer coefficients to zero
	for (int i = 0; i < SH_COEFFICIENT_COUNT; i++) {
		transfer_coeffs[i] = vec3(0.0f, 0.0f, 0.0f);
	}
	
	// Find the starting offset in the TBO
	// Each vertex has a SH_COEFFICIENT_COUNT x SH_COEFFICIENT_COUNT matrix
	int vertex_offset = gl_VertexID * SH_COEFFICIENT_COUNT * SH_COEFFICIENT_COUNT;
	
	// Matrix multiplication, multiply transfer matrix with light coefficients
	for (int j = 0; j < SH_COEFFICIENT_COUNT; j++) {
		for (int i = 0; i < SH_COEFFICIENT_COUNT; i++) {
			transfer_coeffs[j] += texelFetch(tbo_texture, vertex_offset + j * SH_COEFFICIENT_COUNT + i).rgb * light_coeffs[i];
		}
	}
	
	// Obtain reflection direction R using the camera position, using the vertex position/normal
	vec3 to_camera = camera_position - position_in;
	vec3 R = -normalize(reflect(to_camera, normal_in));
	
	// Convert reflection direction R into spherical coordinates (R_theta, R_phi)
	float R_theta       = acos(R.z);
	float inv_sin_theta = 1.0f / sin(R_theta);
	float R_phi         = acos(clamp(R.x * inv_sin_theta, -1.0f, 1.0f));
	
	// Keep phi in the range [0, 2 pi]
	if (R.y * inv_sin_theta < 0.0f) {
        R_phi = 2.0f * pi - R_phi;
	}
	
	vec3 colour = vec3(0.0f, 0.0f, 0.0f);

	int index = 0;
	for (int l = 0; l < SH_NUM_BANDS; l++) {
		for (int m = -l; m <= l; m++) {
			colour += brdf_coeffs[l] * transfer_coeffs[index] * evaluate(l, m, R_theta, R_phi);
			
			index++;
		}
	}

	colour_out = colour;
	
	gl_Position = view_projection * vec4(position_in, 1.0f);
}
