#pragma once
#include <glm/glm.hpp>

#include "Ray.h"

#include "AssetLoader.h"

#include "Types.h"
#include "Util.h"

struct Plane {
	glm::vec3 normal;
	float     distance;
};

struct Triangle {
	Plane     plane;
	glm::vec3 vertices[3];

	bool intersects(const Ray& ray);
};

struct TriangleMesh {
	u32       triangle_count;
	Triangle* triangles;

	TriangleMesh(const AssetLoader::MeshData* mesh_data);

	bool intersects(const Ray& ray);
};