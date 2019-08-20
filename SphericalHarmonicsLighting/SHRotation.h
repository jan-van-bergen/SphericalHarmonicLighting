#pragma once
#include <glm/gtc/quaternion.hpp>

void sh_init_rotation();

void sh_rotate(const glm::quat& rotation, const glm::vec3 coeffs_in[], glm::vec3 coeffs_out[]);
