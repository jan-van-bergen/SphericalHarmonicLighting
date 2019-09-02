#pragma once
#include <glm/glm.hpp>

#include "SphericalHarmonics.h"

class Light {
public:
	glm::vec3 coefficients[SH_COEFFICIENT_COUNT];

	void init(const SH::Sample samples[SAMPLE_COUNT]);

	virtual glm::vec3 get_light(float theta, float phi) const = 0;
};

class DirectionalLight : public Light {
public:
	glm::vec3 get_light(float theta, float phi) const;
};

class HDRProbeLight : public Light {
private:
	int         size;
	glm::vec3 * data;

public:
	HDRProbeLight(const char* filename, int size);
	~HDRProbeLight();

	glm::vec3 get_light(float theta, float phi) const;
};
