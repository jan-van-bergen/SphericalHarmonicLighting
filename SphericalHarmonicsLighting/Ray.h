#pragma once
#include <glm/glm.hpp>

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

	AABB calc_aabb() const;
};

struct Ray {
	glm::vec3 origin;
	glm::vec3 direction;

	bool  intersects(const Triangle& triangle) const;
	float distance  (const Triangle& triangle) const;

	bool intersects(const AABB& aabb) const;
};
