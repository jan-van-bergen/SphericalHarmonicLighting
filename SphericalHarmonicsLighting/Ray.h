#pragma once
#include <glm/glm.hpp>

struct Ray {
	glm::vec3 origin;
	glm::vec3 direction;
};

struct Plane {
	glm::vec3 normal;
	float     distance;
};

struct Triangle {
	Plane     plane;
	glm::vec3 vertices[3];
};

bool ray_triangle_intersect(const Ray& ray, const Triangle& triangle);