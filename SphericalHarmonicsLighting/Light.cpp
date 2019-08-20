#include "Light.h"

#include "Util.h"

void Light::init(int sample_count, const SH_Sample samples[]) {
	// Weighed by the area of a 3D unit sphere
	const float weight = 4.0f * PI;

	// For each sample
	for (int i = 0; i < sample_count; i++) {
		float theta = samples[i].theta;
		float phi   = samples[i].phi;

		// For each SH coefficient
		for (int n = 0; n < SH_COEFFICIENT_COUNT; n++) {
			coefficients[n] += get_light(theta, phi) * samples[i].coeffs[n];
		}
	}

	// Divide the result by weight and number of samples
	const float factor = weight / sample_count;
	for (int i = 0; i < SH_COEFFICIENT_COUNT; i++) {
		coefficients[i] *= factor;
	}
}