#pragma once
#include <glm/glm.hpp>

#include "Types.h"

namespace AssetLoader {
	struct Vertex {
		glm::vec3 position;
		glm::vec2 tex_coord;
		glm::vec3 normal;
	};

	struct MeshData {
		u32      vertex_count;
		Vertex * vertices;

		u32   index_count;
		u32 * indices;
	};

	const MeshData* load_mesh(const char* filename);
}
