#pragma once
#include <glm/glm.hpp>

#define EPSILON 0.001f

// Axis Aligned Bounding Box
struct AABB {
	glm::vec3 min;
	glm::vec3 max;

	void expand(const AABB& other);
};

struct Plane {
	glm::vec3 normal;
	float     distance;
};

struct Triangle {
	Plane     plane;
	glm::vec3 vertices[3];

	int indices[3];

	AABB calc_aabb() const;
};

struct Ray {
	glm::vec3 origin;
	glm::vec3 direction;

	bool  intersects(const Triangle& triangle) const;
	float trace     (const Triangle& triangle, int indices[3], float& u, float& v) const;

	bool intersects(const AABB& aabb) const;
};
