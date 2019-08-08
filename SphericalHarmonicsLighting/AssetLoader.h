#pragma once
#include <string>

#include <glm/glm.hpp>

#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Types.h"

namespace AssetLoader {
	struct Vertex {
		glm::vec3 position;
		glm::vec2 tex_coord;
		glm::vec3 normal;

		inline Vertex() { }

		inline Vertex(const glm::vec3& pos, const glm::vec2& tex, const glm::vec3& nor) {
			position  = pos;
			tex_coord = tex;
			normal    = nor;
		}
	};

	struct MeshData {
		u32     vertex_count;
		Vertex* vertices;

		u32     index_count;
		u32*    indices;
	};

	const MeshData* load_mesh(const char* filename);
}
