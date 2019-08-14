#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

glm::mat4 create_view_matrix(const glm::vec3& camera_position, const glm::quat& camera_rotation);

glm::vec3 min_componentwise(const glm::vec3& a, const glm::vec3& b);
glm::vec3 max_componentwise(const glm::vec3& a, const glm::vec3& b);
