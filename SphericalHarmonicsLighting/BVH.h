#pragma once
#include <GL/glew.h>

#include "Ray.h"

#include "Types.h"

struct BVHNode {
public:
	AABB aabb;
	
	BVHNode const * left;
	BVHNode const * right;

	int               triangle_count;
	Triangle const ** triangles;

	BVHNode();
	~BVHNode();

	bool  intersects(const Ray& ray) const;
	float trace     (const Ray& ray, int indices[3], float& u, float& v) const;

	static BVHNode const * build(int triangle_count, Triangle const * const triangles[]);
};

struct BVHDebugger {
private:
	Array<glm::vec3> positions;
	Array<u32>       indices;

	GLuint vbo;
	GLuint ibo;
	u32 index_count;

	void init_tree(const BVHNode * bvh_node);

public:
	void init(const BVHNode * root);

	void draw() const;
};
