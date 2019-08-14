#pragma once
#include "Ray.h"

#include "Types.h"

class KD_Node {
public:
	AABB aabb;
	
	KD_Node* left;
	KD_Node* right;

	u32               triangle_count;
	Triangle const ** triangles;

	inline KD_Node() {
		aabb.min = glm::vec3(0.0f, 0.0f, 0.0f);
		aabb.max = glm::vec3(0.0f, 0.0f, 0.0f);

		left  = NULL;
		right = NULL;

		triangle_count = 0;
		triangles      = NULL;
	}

	bool intersects(const Ray& ray) const;

	static KD_Node* build(u32 triangle_count, Triangle const * const triangles[]);
};
