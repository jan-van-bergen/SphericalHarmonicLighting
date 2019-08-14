#pragma once
#include "Ray.h"

#include "Types.h"

class KD_Node {
public:
	AABB aabb;
	
	KD_Node* left;
	KD_Node* right;

	Array<Triangle*> triangles;

	static KD_Node* build(const Array<Triangle*>& triangles);
};

KD_Node* KD_Node::build(const Array<Triangle*>& triangles) {
	KD_Node* node = new KD_Node();
	node->aabb.min = glm::vec3(0.0f, 0.0f, 0.0f);
	node->aabb.max = glm::vec3(0.0f, 0.0f, 0.0f);
	node->left  = NULL;
	node->right = NULL;
	node->triangles = triangles;

	// @TODO: should check this in caller?
	if (triangles.size() == 0) return node;

	if (triangles.size() == 1) {
		node->aabb = triangles[0]->calc_aabb();

		return node;
	}

	glm::vec3  center(0.0f, 0.0f, 0.0f);
	glm::vec3* triangle_centers = new glm::vec3[triangles.size()];

	for (int i = 0; i < triangles.size(); i++) {
		node->aabb.expand(triangles[i]->calc_aabb());

		triangle_centers[i] = (triangles[i]->vertices[0] + triangles[i]->vertices[1] + triangles[i]->vertices[2]) * 0.3333333333333333333333f;
		center += triangle_centers[i];
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

	Array<Triangle*> triangles_left;
	Array<Triangle*> triangles_right;

	// @TODO: can we only reserve (triangles.size() >> 1) + 1 here?
	triangles_left.reserve(triangles.size());
	triangles_right.reserve(triangles.size());

	switch (longest_axis) {
		case X_AXIS: {
			for (u32 i = 0; i < triangles.size(); i++) {
				if (triangle_centers[i].x < center.x) {
					triangles_left.push_back(triangles[i]);
				} else {
					triangles_right.push_back(triangles[i]);
				}
			}
		} break;
		
		case Y_AXIS: {
			for (u32 i = 0; i < triangles.size(); i++) {
				if (triangle_centers[i].y < center.y) {
					triangles_left.push_back(triangles[i]);
				} else {
					triangles_right.push_back(triangles[i]);
				}
			}
		} break;

		case Z_AXIS: {
			for (u32 i = 0; i < triangles.size(); i++) {
				if (triangle_centers[i].z < center.z) {
					triangles_left.push_back(triangles[i]);
				} else {
					triangles_right.push_back(triangles[i]);
				}
			}
		} break;
	}
	
	delete[] triangle_centers;

	if (triangles_left.size()  == 0 && triangles_right.size() > 0) triangles_left  = triangles_right;
	if (triangles_right.size() == 0 && triangles_left.size()  > 0) triangles_right = triangles_left;

	// @WTF
	u32 matches = 0;
	for (u32 i = 0; i < triangles_left.size(); i++) {
		for (u32 j = 0; j < triangles_right.size(); j++) {
			if (triangles_left[i] == triangles_right[j]) {
				matches++;
			}
		}
	}

	if (matches <= triangles_left.size() >> 1 && matches <= triangles_right.size() >> 1) {
		node->left  = build(triangles_left);
		node->right = build(triangles_right);
	} else {
		node->left  = new KD_Node();
		node->right = new KD_Node();
	}

	return node;
}
