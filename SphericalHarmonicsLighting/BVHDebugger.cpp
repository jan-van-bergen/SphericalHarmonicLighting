#include "BVH.h"

void BVHDebugger::init_tree(const BVHNode * bvh_node) {
	// Bottom 4 vertices
	positions.push_back(glm::vec3(bvh_node->aabb.min.x, bvh_node->aabb.min.y, bvh_node->aabb.min.z));
	positions.push_back(glm::vec3(bvh_node->aabb.max.x, bvh_node->aabb.min.y, bvh_node->aabb.min.z));
	positions.push_back(glm::vec3(bvh_node->aabb.max.x, bvh_node->aabb.min.y, bvh_node->aabb.max.z));
	positions.push_back(glm::vec3(bvh_node->aabb.min.x, bvh_node->aabb.min.y, bvh_node->aabb.max.z));
	// Top 4 vertices
	positions.push_back(glm::vec3(bvh_node->aabb.min.x, bvh_node->aabb.max.y, bvh_node->aabb.min.z));
	positions.push_back(glm::vec3(bvh_node->aabb.max.x, bvh_node->aabb.max.y, bvh_node->aabb.min.z));
	positions.push_back(glm::vec3(bvh_node->aabb.max.x, bvh_node->aabb.max.y, bvh_node->aabb.max.z));
	positions.push_back(glm::vec3(bvh_node->aabb.min.x, bvh_node->aabb.max.y, bvh_node->aabb.max.z));
	
	int index_start = indices.size();

	// Bottom of AABB
	indices.push_back(index_start + 0); indices.push_back(index_start + 1); 
	indices.push_back(index_start + 1); indices.push_back(index_start + 2); 
	indices.push_back(index_start + 2); indices.push_back(index_start + 3); 
	indices.push_back(index_start + 3); indices.push_back(index_start + 0); 
	// Top of AABB
	indices.push_back(index_start + 4); indices.push_back(index_start + 5); 
	indices.push_back(index_start + 5); indices.push_back(index_start + 6); 
	indices.push_back(index_start + 6); indices.push_back(index_start + 7); 
	indices.push_back(index_start + 7); indices.push_back(index_start + 4); 
	// Edges between bottom and top of AABB
	indices.push_back(index_start + 0); indices.push_back(index_start + 4); 
	indices.push_back(index_start + 1); indices.push_back(index_start + 5); 
	indices.push_back(index_start + 2); indices.push_back(index_start + 6); 
	indices.push_back(index_start + 3); indices.push_back(index_start + 7); 

	// Recurse if the node has children
	if (bvh_node->left) {
		init_tree(bvh_node->left);
		init_tree(bvh_node->right);
	}
}

void BVHDebugger::init(const BVHNode * root) {
	init_tree(root);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(u32), indices.data(), GL_STATIC_DRAW);

	index_count = indices.size();

	positions.clear();
	indices.clear();
}

void BVHDebugger::draw() const {
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glDrawElements(GL_LINES, index_count, GL_UNSIGNED_INT, NULL);

	glDisableVertexAttribArray(0);
}
