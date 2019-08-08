#pragma once
#include "AssetLoader.h"
#include "TriangleMesh.h"
#include "Ray.h"

#include "Types.h"
#include "Util.h"

class Scene
{
public:
	inline Scene() {
		TriangleMesh mesh = TriangleMesh(AssetLoader::load_mesh(DATA_PATH("box.obj")));
		triangle_meshes.emplace_back(mesh);

		Ray ray = { glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, -1.0f) };

	}

private:
	Array<TriangleMesh> triangle_meshes;
};

