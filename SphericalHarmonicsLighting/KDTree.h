#pragma once
#include <GL/glew.h>

#include "Ray.h"

#include "Types.h"

struct KD_Node {
public:
	AABB aabb;
	
	KD_Node * left;
	KD_Node * right;

	int               triangle_count;
	Triangle const ** triangles;

	KD_Node();
	~KD_Node();

	bool intersects(const Ray& ray) const;

	static KD_Node* build(int triangle_count, Triangle const * const triangles[]);
};

struct KD_Node_Debugger {
private:
	Array<glm::vec3> positions;
	Array<u32>       indices;

	GLuint vbo;
	GLuint ibo;
	u32 index_count;

	void init_tree(const KD_Node * kd_node);

public:
	void init(const KD_Node * root);

	void draw() const;
};