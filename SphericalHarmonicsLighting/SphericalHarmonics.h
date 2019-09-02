#pragma once
#include <glm/glm.hpp>

// Number of Spherical Harmonic bands, commonly referred to with the letter l
#define SH_NUM_BANDS 5
#define SH_COEFFICIENT_COUNT (SH_NUM_BANDS * SH_NUM_BANDS)

// Amount of samples used for Monte Carlo integration
#define SQRT_SAMPLE_COUNT 50
#define SAMPLE_COUNT (SQRT_SAMPLE_COUNT * SQRT_SAMPLE_COUNT)

namespace SH {
	typedef glm::vec3 (*PolarFunction)(float theta, float phi);

	// Spherical Harmonics Sample
	struct Sample {
		// Sample direction, in sperical coordinates as well as cartesian coordinates
		float theta;
		float phi;
		glm::vec3 direction;

		// SH coefficients that make up the sample
		float coeffs[SH_COEFFICIENT_COUNT];
	};

	void init_samples(Sample samples[SAMPLE_COUNT]);

	void project_polar_function(PolarFunction fn, int n_samples, const Sample samples[], glm::vec3 result[]);

	void calc_phong_lobe_coeffs(float result[SH_NUM_BANDS]);
}
