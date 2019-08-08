#pragma once
#include <map>

#include <assimp/Importer.hpp> 

#include "AssetLoader.h"

namespace AssetLoader {
	std::unordered_map<const char*, const MeshData*> mesh_cache;
	
	const MeshData* init_mesh_data(const aiMesh* mesh) {
		MeshData* mesh_data = new MeshData();
		mesh_data->vertex_count = mesh->mNumVertices;
		mesh_data->index_count  = mesh->mNumFaces * 3;

		mesh_data->vertices = new Vertex[mesh_data->vertex_count];
		mesh_data->indices  = new u32   [mesh_data->index_count];

		assert(mesh->HasTextureCoords(0));

		for (u32 i = 0; i < mesh->mNumVertices; i++) {
			const aiVector3D& pos = mesh->mVertices[i];
			const aiVector3D& nor = mesh->mNormals[i];
			const aiVector3D& tex = mesh->mTextureCoords[0][i];

			mesh_data->vertices[i] = Vertex(
				glm::vec3(pos.x, pos.y, pos.z),
				glm::vec2(tex.x, tex.y),
				glm::vec3(nor.x, nor.y, nor.z)
			);
		}

		for (u32 i = 0; i < mesh->mNumFaces; i++) {
			const aiFace& face = mesh->mFaces[i];

			assert(face.mNumIndices == 3);

			mesh_data->indices[i*3    ] = face.mIndices[0];
			mesh_data->indices[i*3 + 1] = face.mIndices[1];
			mesh_data->indices[i*3 + 2] = face.mIndices[2];
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
