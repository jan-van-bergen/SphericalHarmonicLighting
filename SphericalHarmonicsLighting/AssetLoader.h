#pragma once
#include <string>

#include <glm/glm.hpp>

#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Types.h"

namespace AssetLoader {
	struct Vertex {
		glm::vec3 m_pos;
		glm::vec2 m_tex;
		glm::vec3 m_normal;

		inline Vertex() { }

		inline Vertex(const glm::vec3& pos, const glm::vec2& tex, const glm::vec3& normal) {
			m_pos = pos;
			m_tex = tex;
			m_normal = normal;
		}
	};

	struct MeshData {
		std::vector<Vertex> vertices;
		std::vector<u32>    indices;
	};

	const MeshData* load_mesh(const char* filename);
}
