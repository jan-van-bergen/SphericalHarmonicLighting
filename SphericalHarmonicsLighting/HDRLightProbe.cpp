#include "HDRLightProbe.h"

#include <fstream>

#include "Util.h"

struct HDR_Image {
	u32        size;
	glm::vec3* data;
} image;

void load_hdr_image(const char* filename, u32 size) {
	image.size = size;
	image.data = new glm::vec3[size * size];

	std::ifstream file(filename, std::ios::in | std::ios::binary); 
	if (!file.is_open()) {
		abort();
	} 

	file.read(reinterpret_cast<char*>(image.data), size * size * sizeof(glm::vec3));
	file.close();
}

glm::vec3 sample_hdr_image(float theta, float phi) {
	const glm::vec3 direction(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));

	const float r = (1.0f / PI) * acos(direction.z) / sqrt(direction.x*direction.x + direction.y*direction.y);

	const float u = direction.x * r;
	const float v = direction.y * r;

	const u32 x = (u * 0.5f + 0.5f) * (float)image.size;
	const u32 y = (v * 0.5f + 0.5f) * (float)image.size;

	return image.data[x * image.size + y];
}
