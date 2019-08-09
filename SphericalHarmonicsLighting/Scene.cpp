#include "Scene.h"

bool Triangle::intersects(const Ray& ray) const {
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
		const glm::vec3 v_a = vertices[i        ] - intersection_point;
		const glm::vec3 v_b = vertices[(i+1) % 3] - intersection_point;

		// Calculate the angle between these vectors
		angle += acos(glm::clamp(glm::dot(v_a, v_b) / (glm::length(v_a) * glm::length(v_b)), -1.0f, 1.0f));
	}

	// If the sum of the angles is greater than 2 pi radians, then the point is inside the triangle
	return angle >= 1.99f * PI;
}
