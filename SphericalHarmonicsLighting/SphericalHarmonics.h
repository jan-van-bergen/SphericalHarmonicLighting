#pragma once
#include <glm/glm.hpp>

#include "Util.h"

// Number of Spherical Harmonic bands, commonly referred to with the letter l
#define SH_NUM_BANDS 5
#define SH_COEFFICIENT_COUNT (SH_NUM_BANDS * SH_NUM_BANDS)

// Amount of samples used for Monte Carlo integration
#define SQRT_SAMPLE_COUNT 50
#define SAMPLE_COUNT (SQRT_SAMPLE_COUNT * SQRT_SAMPLE_COUNT)

namespace SH {
	// Spherical Harmonics Sample
	struct Sample {
		// Sample direction, in sperical coordinates as well as cartesian coordinates
		float theta;
		float phi;
		glm::vec3 direction;

		// SH coefficients that make up the sample
		float coeffs[SH_COEFFICIENT_COUNT];
	};

	// Returns a point sample of a Spherical Harmonic basis function
	// l is the band, range [0..N]
	// m in the range [-l..l]
	// theta in the range [0..Pi]
	// phi in the range [0..2*Pi]
	float evaluate(int l, int m, float theta, float phi);
	
	// Fills the sample array with uniformly distributed SH samples across the unit sphere, using jittered stratification
	void init_samples(Sample samples[SAMPLE_COUNT]);

	// Projects a given polar function into Spherical Harmonic coefficients.
	// This is done using Monte Carlo integration, using the samples provided in the samples array
	template<typename PolarFunction>
	void project_polar_function(PolarFunction& polar_function, const Sample samples[SAMPLE_COUNT], glm::vec3 result[]) {
		// For each sample
		for (int s = 0; s < SAMPLE_COUNT; s++) {
			float theta = samples[s].theta;
			float phi   = samples[s].phi;

			// For each SH coefficient
			for (int c = 0; c < SH_COEFFICIENT_COUNT; c++) {
				result[c] += polar_function(theta, phi) * samples[s].coeffs[c];
			}
		}

		//  Weighted by the surface area of a 3D unit sphere, divided by the number of samples
		const float factor = 4.0f * PI / SAMPLE_COUNT;
		for (int c = 0; c < SH_COEFFICIENT_COUNT; c++) {
			result[c] *= factor;
		}
	}
	
	// Calulates phong lobe SH coefficients using an analytical approxiamation
	// Formula from the paper "An Efficient Representation for Irradiance Environment Maps" by Ramamoorthi and Hanrahan
	void calc_phong_lobe_coeffs(float result[SH_NUM_BANDS]);
}
