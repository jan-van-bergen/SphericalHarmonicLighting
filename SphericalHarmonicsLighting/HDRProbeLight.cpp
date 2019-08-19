#include "Light.h"

#include <fstream>

#include "Util.h"

HDRProbeLight::HDRProbeLight(const char* filename, int size) : size(size), data(new glm::vec3[size * size]) {
	std::ifstream file(filename, std::ios::in | std::ios::binary); 
	if (!file.is_open()) {
		abort();
	} 

	file.read(reinterpret_cast<char*>(data), size * size * sizeof(glm::vec3));
	file.close();
}

glm::vec3 HDRProbeLight::get_light(float theta, float phi) const {
	const glm::vec3 direction(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));

	const float r = (1.0f / PI) * acos(direction.z) / sqrt(direction.x*direction.x + direction.y*direction.y);

	const float u = direction.x * r;
	const float v = direction.y * r;

	const int x = (u * 0.5f + 0.5f) * size;
	const int y = (v * 0.5f + 0.5f) * size;

	assert(data);
	return data[x * size + y];
}
