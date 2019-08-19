#include "Ray.h"

#include <algorithm>

#include "VectorMath.h"

#include "Types.h"
#include "Util.h"

void AABB::expand(const AABB& other) {
	min = min_componentwise(min, other.min);
	max = max_componentwise(max, other.max);
}

AABB Triangle::calc_aabb() const {
	AABB aabb;

	aabb.min = min_componentwise(min_componentwise(vertices[0], vertices[1]), vertices[2]);
	aabb.max = max_componentwise(max_componentwise(vertices[0], vertices[1]), vertices[2]);

	return aabb;
}

bool Ray::intersects(const Triangle& triangle) const {
	float dot = glm::dot(direction, triangle.plane.normal);

	// If the plane's normal is orthogonal to the Ray's direction there can be no intersection
	if (abs(dot) < 0.001f) return false;

	// Get the ray parameter where it meets the plane
	float t = -(glm::dot(origin, triangle.plane.normal) + triangle.plane.distance) / dot;

	// If t is negative the intersection takes place behind the Ray
	if (t <= 0.0f) return false;

	// Calculate the point of intersection between the Ray and Plane
	glm::vec3 intersection_point = origin + t * direction;

	glm::vec3 v0 = triangle.vertices[0] - intersection_point;
	glm::vec3 v1 = triangle.vertices[1] - intersection_point;
	glm::vec3 v2 = triangle.vertices[2] - intersection_point;

	float length0 = glm::length(v0);
	float length1 = glm::length(v1);
	float length2 = glm::length(v2);

	// Calculate the angles between these vectors
	float angle = 
		acos(glm::clamp(glm::dot(v0, v1) / (length0 * length1), -1.0f, 1.0f)) +
		acos(glm::clamp(glm::dot(v1, v2) / (length1 * length2), -1.0f, 1.0f)) +
		acos(glm::clamp(glm::dot(v2, v0) / (length2 * length0), -1.0f, 1.0f));

	// If the sum of the angles is greater than 2 pi radians, then the point is inside the triangle
	return angle >= 1.99f * PI;
}

float Ray::distance(const Triangle& triangle) const {
	float dot = glm::dot(direction, triangle.plane.normal);

	// If the plane's normal is orthogonal to the Ray's direction there can be no intersection
	if (abs(dot) < 0.001f) return false;

	// Get the ray parameter where it meets the plane
	float t = -(glm::dot(origin, triangle.plane.normal) + triangle.plane.distance) / dot;

	// If t is negative the intersection takes place behind the Ray
	if (t <= 0.0f) return false;

	// Calculate the point of intersection between the Ray and Plane
	glm::vec3 intersection_point = origin + t * direction;

	// Check if the intersection point is inside the triangle
	float angle = 0.0f;

	for (int i = 0; i < 3; i++)
	{
		// Calculate the vector from this vertex to the intersection point
		const glm::vec3 v_a = triangle.vertices[i        ] - intersection_point;
		const glm::vec3 v_b = triangle.vertices[(i+1) % 3] - intersection_point;

		// Calculate the angle between these vectors
		angle += acos(glm::clamp(glm::dot(v_a, v_b) / (glm::length(v_a) * glm::length(v_b)), -1.0f, 1.0f));
	}

	// If the sum of the angles is greater than 2 pi radians, then the point is inside the triangle
	if (angle >= 1.99f * PI) {
		return t;
	} else {
		return INFINITY;
	}
}

bool Ray::intersects(const AABB& aabb) const {
	glm::vec3 inv_direction(1.0f / direction.x, 1.0f / direction.y, 1.0f / direction.z);

    float t1 = (aabb.min.x - origin.x) * inv_direction.x;
    float t2 = (aabb.max.x - origin.x) * inv_direction.x;
    float t3 = (aabb.min.y - origin.y) * inv_direction.y;
    float t4 = (aabb.max.y - origin.y) * inv_direction.y;
    float t5 = (aabb.min.z - origin.z) * inv_direction.z;
    float t6 = (aabb.max.z - origin.z) * inv_direction.z;

    float t_min = std::max(std::max(std::min(t1, t2), std::min(t3, t4)), std::min(t5, t6));
    float t_max = std::min(std::min(std::max(t1, t2), std::max(t3, t4)), std::max(t5, t6));

    return t_max >= 0.0f && t_min < t_max;
}
