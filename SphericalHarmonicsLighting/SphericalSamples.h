#pragma once
#include <glm/glm.hpp>

#define SH_NUM_BANDS 5
#define SH_COEFFICIENT_COUNT (SH_NUM_BANDS * SH_NUM_BANDS)

typedef glm::vec3 (*PolarFunction)(float theta, float phi);

// Spherical Harmonics Sample
struct SH_Sample {
	float theta;
	float phi;
	glm::vec3 direction;

	float coeffs[SH_COEFFICIENT_COUNT];
};

void init_samples(SH_Sample samples[], int sqrt_n_samples, int n_bands);

void project_polar_function(PolarFunction fn, int n_samples, const SH_Sample samples[], glm::vec3 result[]);

void calc_phong_lobe_coeffs(float result[SH_NUM_BANDS]);
