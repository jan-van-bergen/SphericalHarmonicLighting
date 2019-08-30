#pragma once
#include <glm/gtc/quaternion.hpp>

namespace SH {
	void init_rotation();

	void rotate(const glm::quat& rotation, const glm::vec3 coeffs_in[], glm::vec3 coeffs_out[]);
}
