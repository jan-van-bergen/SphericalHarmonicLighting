#pragma once
#include <glm/glm.hpp>

#include "Types.h"

typedef float (*PolarFunction)(float theta, float phi);

// Spherical Harmonics Sample
template<u32 Degree>
struct SHSample {
	float theta;
	float phi;
	glm::vec3 direction;

	float coeffs[Degree];
};

template<u32 Degree>
void setup_spherical_samples(SHSample<Degree> samples[], u32 sqrt_n_samples, u32 n_bands);

template<u32 Degree>
void project_polar_function(PolarFunction fn, u32 n_samples, const SHSample<Degree> samples[], float result[]);
