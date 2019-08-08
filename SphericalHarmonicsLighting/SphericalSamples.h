#pragma once
#include <glm/glm.hpp>

#include "Types.h"

#define SH_NUM_BANDS 5
#define SH_COEFFICIENT_COUNT (SH_NUM_BANDS * SH_NUM_BANDS)

typedef float (*PolarFunction)(float theta, float phi);

// Spherical Harmonics Sample
struct SH_Sample {
	float theta;
	float phi;
	glm::vec3 direction;

	float coeffs[SH_COEFFICIENT_COUNT];
};

void init_samples(SH_Sample samples[], u32 sqrt_n_samples, u32 n_bands);

template<u32 Degree>
void project_polar_function(PolarFunction fn, u32 n_samples, const SH_Sample samples[], float result[]);
