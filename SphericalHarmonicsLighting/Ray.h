#pragma once
#include <glm/glm.hpp>

#include "TriangleMesh.h"

struct Ray {
	glm::vec3 origin;
	glm::vec3 direction;
};


bool ray_triangle_intersect(const Ray& ray, const Triangle& triangle);