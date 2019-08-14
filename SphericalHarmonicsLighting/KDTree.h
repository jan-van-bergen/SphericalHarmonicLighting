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

	bool intersects(const Ray& ray) const;

	static KD_Node* build(u32 triangle_count, Triangle const * const triangles[]);
};
