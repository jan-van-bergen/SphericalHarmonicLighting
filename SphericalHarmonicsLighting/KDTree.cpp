#include "KDTree.h"

#define TERMINATION_SIZE 2

KD_Node::KD_Node() {
	aabb.min = glm::vec3(+INFINITY);
	aabb.max = glm::vec3(-INFINITY);

	left  = NULL;
	right = NULL;

	triangle_count = 0;
	triangles      = NULL;
}

KD_Node::~KD_Node() {
	if (left) {
		delete left;
		delete right;
	}

	if (triangles) {
		delete[] triangles;
	}
}

bool KD_Node::intersects(const Ray& ray) const {
	if (ray.intersects(aabb)) {
		if (left) { // If the left node pointer is non-null, we are not in a leaf node and need to recurse
			assert(triangles == NULL);

			return left->intersects(ray) || right->intersects(ray);
		} else { // The current node is a leaf
			assert(left  == NULL);
			assert(right == NULL);

			for (int i = 0; i < triangle_count; i++) {
				if (ray.intersects(*triangles[i])) {
					return true;
				}
			}
		}
	}

	return false;
}

KD_Node * KD_Node::build(int triangle_count, Triangle const * const triangles[]) {
	KD_Node * node = new KD_Node();
	node->triangle_count = triangle_count;

	// @TODO: should check this in caller?
	if (triangle_count == 0) return node;

	if (triangle_count == 1) {
		node->aabb = triangles[0]->calc_aabb();

		node->triangles    = new Triangle const *[1];
		node->triangles[0] = triangles[0];

		return node;
	}

	// Check termination condition
	if (triangle_count <= TERMINATION_SIZE) {
		node->triangles = new Triangle const *[triangle_count];
		memcpy(node->triangles, triangles, triangle_count * sizeof(Triangle *));

		return node;
	}

	glm::vec3  center(0.0f, 0.0f, 0.0f);
	// @PERFORMANCE: should be calculated only once and passed as an argument,
	// instead of recalulcating at every depth level in the tree
	glm::vec3* triangle_centers = new glm::vec3[triangle_count]; 

	const float inv_triangle_count = 1.0f / (float)triangle_count;
	
	for (int i = 0; i < triangle_count; i++) {
		node->aabb.expand(triangles[i]->calc_aabb());

		triangle_centers[i] = (triangles[i]->vertices[0] + triangles[i]->vertices[1] + triangles[i]->vertices[2]) * 0.3333333333333333333333f;
		center += triangle_centers[i] * inv_triangle_count;
	}
	
	enum Axis { X_AXIS = 0, Y_AXIS = 1, Z_AXIS = 2 } longest_axis = X_AXIS;

	float size_x = node->aabb.max.x - node->aabb.min.x;
	float size_y = node->aabb.max.y - node->aabb.min.y;
	float size_z = node->aabb.max.z - node->aabb.min.z;

	if (size_y > size_x) {
		if (size_z > size_y) {
			longest_axis = Z_AXIS;
		} else {
			longest_axis = Y_AXIS;
		}
	} else if (size_z > size_x) {
		longest_axis = Z_AXIS;
	}

	// Allocate a new buffer for the Triangle pointers to be copied into
	// The buffer grows left to right, containing the Triangles left of the average
	// The buffer grows right to left, containing the Triangles right of the average
	Triangle const ** triangle_buffer = new Triangle const *[triangle_count];

	// Indices indicating where the two buffers growing towards eachother currently are
	int triangle_index_left  = 0;
	int triangle_index_right = triangle_count - 1;

	// Split along the longest axis
	switch (longest_axis) {
		case X_AXIS: {
			for (int i = 0; i < triangle_count; i++) {
				if (triangle_centers[i].x < center.x) {
					triangle_buffer[triangle_index_left++]  = triangles[i];
				} else {
					triangle_buffer[triangle_index_right--] = triangles[i];
				}
			}
		} break;
		
		case Y_AXIS: {
			for (int i = 0; i < triangle_count; i++) {
				if (triangle_centers[i].y < center.y) {
					triangle_buffer[triangle_index_left++]  = triangles[i];
				} else {
					triangle_buffer[triangle_index_right--] = triangles[i];
				}
			}
		} break;

		case Z_AXIS: {
			for (int i = 0; i < triangle_count; i++) {
				if (triangle_centers[i].z < center.z) {
					triangle_buffer[triangle_index_left++]  = triangles[i];
				} else {
					triangle_buffer[triangle_index_right--] = triangles[i];
				}
			}
		} break;
	}

	// Sanity check, at the end the left and right buffers should meet exactly
	assert(triangle_index_left == triangle_index_right + 1);
	
	delete[] triangle_centers;
	
	// Recurse
	node->left  = build(triangle_index_left,                  triangle_buffer);
	node->right = build(triangle_count - triangle_index_left, triangle_buffer + triangle_index_left);
	
	delete[] triangle_buffer;

	return node;
}
