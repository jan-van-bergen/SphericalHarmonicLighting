#pragma once
#include <glm/glm.hpp>

#include "Types.h"
#include "AssetLoader.h"

struct Plane {
	glm::vec3 normal;
	float     distance;
};

struct Triangle {
	Plane     plane;
	glm::vec3 vertices[3];
};

struct TriangleMesh {
	u32       triangle_count;
	Triangle* triangles;

	inline TriangleMesh(const AssetLoader::MeshData* mesh_data) {
		assert(mesh_data->index_count % 3 == 0);

		triangle_count = mesh_data->index_count / 3;
		triangles      = new Triangle[triangle_count];

		for (u32 i = 0; i < triangle_count; i++) {
			// Set the three Vertex positions of the Triangle
			triangles[i].vertices[0] = mesh_data->vertices[mesh_data->indices[3*i    ]].position;
			triangles[i].vertices[1] = mesh_data->vertices[mesh_data->indices[3*i + 1]].position;
			triangles[i].vertices[2] = mesh_data->vertices[mesh_data->indices[3*i + 2]].position;

			// Calculate the Plane equation that fits the Triangle
			glm::vec3 edge0 = triangles[i].vertices[1] - triangles[i].vertices[0];
			glm::vec3 edge1 = triangles[i].vertices[2] - triangles[i].vertices[0];

			triangles[i].plane.normal   =  glm::normalize(glm::cross(edge0, edge1));
			triangles[i].plane.distance = -glm::dot(triangles[i].plane.normal, triangles[i].vertices[0]);
		}
	}
};