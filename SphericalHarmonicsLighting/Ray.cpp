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
	// @PERFORMANCE
	glm::vec3 e0 = triangle.vertices[1] - triangle.vertices[0];
	glm::vec3 e1 = triangle.vertices[2] - triangle.vertices[0];

	glm::vec3 h = glm::cross(direction, e1);
	float a = glm::dot(e0, h);

	float f = 1.0f / a;
	glm::vec3 s = origin - triangle.vertices[0];
	float _u = f * glm::dot(s, h);

	if (_u < 0.0f || _u > 1.0f) return false;

	glm::vec3 q = glm::cross(s, e0);
	float _v = f * glm::dot(direction, q);

	if (_v < 0.0f || _u + _v > 1.0f) return false;

	float t = f * glm::dot(e1, q);

	if (t <= EPSILON) return false;
	
	return true;
}

float Ray::trace(const Triangle& triangle, int indices[3], float& u, float& v) const {
	// @PERFORMANCE
	glm::vec3 e0 = triangle.vertices[1] - triangle.vertices[0];
	glm::vec3 e1 = triangle.vertices[2] - triangle.vertices[0];

	glm::vec3 h = glm::cross(direction, e1);
	float a = glm::dot(e0, h);
	
	float f = 1.0f / a;
	glm::vec3 s = origin - triangle.vertices[0];
	float _u = f * glm::dot(s, h);

	if (_u < 0.0f || _u > 1.0f) return INFINITY;

	glm::vec3 q = glm::cross(s, e0);
	float _v = f * glm::dot(direction, q);

	if (_v < 0.0f || _u + _v > 1.0f) return INFINITY;

	float t = f * glm::dot(e1, q);

	if (t <= EPSILON) return INFINITY;

	memcpy(indices, triangle.indices, 3 * sizeof(int));
	u = _u;
	v = _v;

	return t;
}

bool Ray::intersects(const AABB& aabb) const {
	float inv_direction_x = 1.0f / direction.x;
	float inv_direction_y = 1.0f / direction.y;
	float inv_direction_z = 1.0f / direction.z;

	float tmin = (aabb.min.x - origin.x) * inv_direction_x; 
    float tmax = (aabb.max.x - origin.x) * inv_direction_x; 
 
    if (tmin > tmax) std::swap(tmin, tmax); 
 
    float tymin = (aabb.min.y - origin.y) * inv_direction_y; 
    float tymax = (aabb.max.y - origin.y) * inv_direction_y; 
 
    if (tymin > tymax) std::swap(tymin, tymax); 
 
    if ((tmin > tymax) || (tymin > tmax)) return false; 
 
    if (tymin > tmin) tmin = tymin; 
    if (tymax < tmax) tmax = tymax; 
 
    float tzmin = (aabb.min.z - origin.z) * inv_direction_z; 
    float tzmax = (aabb.max.z - origin.z) * inv_direction_z; 
 
    if (tzmin > tzmax) std::swap(tzmin, tzmax); 
 
    if ((tmin > tzmax) || (tzmin > tmax)) return false; 
 
    return true;
}
