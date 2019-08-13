#pragma once
#include <glm/glm.hpp>

#include "SphericalSamples.h"

#include "Types.h"
#include "Util.h"

class Light {
public:
	glm::vec3 coefficients[SH_COEFFICIENT_COUNT];

	void init(u32 sample_count, const SH_Sample samples[]) {
		// Weighed by the area of a 3D unit sphere
		const float weight = 4.0f * PI;

		// For each sample
		for (u32 i = 0; i < sample_count; i++) {
			float theta = samples[i].theta;
			float phi   = samples[i].phi;

			// For each SH coefficient
			for (u32 n = 0; n < SH_COEFFICIENT_COUNT; n++) {
				coefficients[n] += get_light(theta, phi) * samples[i].coeffs[n];
			}
		}

		// Divide the result by weight and number of samples
		const float factor = weight / sample_count;
		for (u32 i = 0; i < SH_COEFFICIENT_COUNT; i++) {
			coefficients[i] *= factor;
		}
	}

	virtual glm::vec3 get_light(float theta, float phi) const = 0;
};

class DirectionalLight : public Light {
public:
	glm::vec3 get_light(float theta, float phi) const;
};

class HDRProbeLight : public Light {
private:
	u32        size;
	glm::vec3* data;

public:
	HDRProbeLight(const char* filename, u32 size);

	glm::vec3 get_light(float theta, float phi) const;
};
