#include "TriangleMesh.h"

bool Triangle::intersects(const Ray& ray) {
	float dot = glm::dot(ray.direction, plane.normal);

	// If the plane's normal is orthogonal to the Ray's direction there can be no intersection
	if (abs(dot) < 0.001f) return false;

	// Get the ray parameter where it meets the plane
	float t = -(glm::dot(ray.origin, plane.normal) + plane.distance) / dot;

	// If t is negative the intersection takes place behind the Ray
	if (t <= 0.0f) return false;

	// Calculate the point of intersection between the Ray and Plane
	glm::vec3 intersection_point = ray.origin + t * ray.direction;

	// Check if the intersection point is inside the triangle
	float angle = 0.0f;

	// @SPEED: unroll loop
	for (u32 i = 0; i < 3; i++)
	{
		// Calculate the vector from this vertex to the intersection point
		const glm::vec3& v_a = vertices[i        ] - intersection_point;
		const glm::vec3& v_b = vertices[(i+1) % 3] - intersection_point;

		// Calculate the angle between these vectors
		angle += acos(glm::clamp(glm::dot(v_a, v_b) / (glm::length(v_a) * glm::length(v_b)), -1.0f, 1.0f));
	}

	// If the sum of the angles is greater than 2 pi radians, then the point is inside the triangle
	return angle >= 1.99f * PI;
}


TriangleMesh::TriangleMesh(const AssetLoader::MeshData* mesh_data) {
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

bool TriangleMesh::intersects(const Ray& ray) {
	// If any Triangle intersects the Ray then the Triangle Mesh intersects the Ray as well
	for (u32 i = 0; i < triangle_count; i++) {
		if (triangles[i].intersects(ray)) {
			return true;
		}
	}

	// If none of the Triangles intersect the nthe Triangle Mesh doesn't either
	return false;
}