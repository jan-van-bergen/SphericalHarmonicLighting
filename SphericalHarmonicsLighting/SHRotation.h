#pragma once
#include <glm/gtc/quaternion.hpp>

void rotate(const glm::quat& rotation, const glm::vec3 coeffs_in[], glm::vec3 coeffs_out[]);