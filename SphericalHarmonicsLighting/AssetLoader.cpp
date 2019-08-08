#pragma once
#include <map>

#include <assimp/Importer.hpp> 

#include "AssetLoader.h"

namespace AssetLoader {
	std::unordered_map<const char*, const MeshData*> mesh_cache;
	
	const MeshData* init_mesh_data(const aiMesh* mesh) {
		MeshData* mesh_data = new MeshData();

		const aiVector3D zero(0.0f, 0.0f, 0.0f);

		for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
			const aiVector3D& pos = mesh->mVertices[i];
			const aiVector3D& nor = mesh->mNormals[i];
			const aiVector3D& tex = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][i] : zero;

			Vertex v(
				glm::vec3(pos.x, pos.y, pos.z),
				glm::vec2(tex.x, tex.y),
				glm::vec3(nor.x, nor.y, nor.z)
			);

			mesh_data->vertices.emplace_back(v);
		}

		for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
			const aiFace& face = mesh->mFaces[i];

			assert(face.mNumIndices == 3);

			mesh_data->indices.push_back(face.mIndices[0]);
			mesh_data->indices.push_back(face.mIndices[1]);
			mesh_data->indices.push_back(face.mIndices[2]);
		}

		return mesh_data;
	}

	const MeshData* load_new_mesh(const char* filename) {
		Assimp::Importer Importer;
		const aiScene* scene = Importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);

		if (scene) {
			if (scene->mNumMeshes > 1) {
				printf("Warning! multiple meshes are not supported! File: %s", filename);
			}

			return init_mesh_data(scene->mMeshes[0]);
		} else {
			printf("Error parsing '%s': '%s'\n", filename, Importer.GetErrorString());

			abort();
		}
	}

	const MeshData* load_mesh(const char* filename) {
		const MeshData*& mesh_data = mesh_cache[filename];

		if (mesh_data) {
			return mesh_data;
		}

		mesh_data = load_new_mesh(filename);
		return mesh_data;
	}
}
