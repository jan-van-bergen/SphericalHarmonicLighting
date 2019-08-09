#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

inline glm::mat4 create_view_matrix(const glm::vec3& camera_position, const glm::quat& camera_rotation) {
	const glm::vec3 u(camera_rotation.x, camera_rotation.y, camera_rotation.z);
	const float     s = camera_rotation.w;

	const float dot = (s * s - glm::dot(u, u));

	const glm::vec3 right(
		2.0f * u.x * u.x + dot,
		2.0f * u.x * u.y + 2.0f * s * u.z,
		2.0f * u.x * u.z - 2.0f * s * u.y
	); // camera_rotation * glm::vec3(1.0f, 0.0f,  0.0f);
	const glm::vec3 up(
		2.0f * u.y * u.x - 2.0f * s * u.z,
		2.0f * u.y * u.y + dot,
		2.0f * u.y * u.z + 2.0f * s * u.x
	); // camera_rotation * glm::vec3(0.0f, 1.0f,  0.0f);
	const glm::vec3 forward(
		-2.0f * u.z * u.x - 2.0f * s * u.y,
		-2.0f * u.z * u.y + 2.0f * s * u.x,
		-2.0f * u.z * u.z - dot
	); // camera_rotation * glm::vec3(0.0f, 0.0f, -1.0f);

	glm::mat4 result;
	{
		result[0][0] =  right.x;   result[1][0] =  right.y;   result[2][0] =  right.z;   result[3][0] = -glm::dot(camera_position, right);
		result[0][1] =  up.x;      result[1][1] =  up.y;      result[2][1] =  up.z;      result[3][1] = -glm::dot(camera_position, up);
		result[0][2] = -forward.x; result[1][2] = -forward.y; result[2][2] = -forward.z; result[3][2] =  glm::dot(camera_position, forward);
		result[0][3] =  0.0f;      result[1][3] =  0.0f;      result[2][3] =  0.0f;      result[3][3] =  1.0f;
	}
	return result;
}