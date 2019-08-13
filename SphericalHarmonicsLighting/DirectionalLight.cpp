#include "Light.h"

#include "Util.h"

glm::vec3 DirectionalLight::get_light(float theta, float phi) const {
	return (theta < PI / 6.0f) 
		? glm::vec3(1.0f, 1.0f, 1.0f) 
		: glm::vec3(0.0f, 0.0f, 0.0f);
}
