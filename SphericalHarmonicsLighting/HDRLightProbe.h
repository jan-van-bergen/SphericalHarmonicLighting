#pragma once

#include <glm/glm.hpp>

#include "Types.h"

void load_hdr_image(const char* filename, u32 size);

glm::vec3 sample_hdr_image(float theta, float phi);
