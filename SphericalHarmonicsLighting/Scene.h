#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>

#include "Ray.h"

#include "AssetLoader.h"

#include "SphericalSamples.h"

#include "Types.h"
#include "Util.h"

#define SQRT_SAMPLE_COUNT 50
#define SAMPLE_COUNT      (SQRT_SAMPLE_COUNT * SQRT_SAMPLE_COUNT)

struct Plane {
	glm::vec3 normal;
	float     distance;
};

struct Triangle {
	Plane     plane;
	glm::vec3 vertices[3];

	bool intersects(const Ray& ray) const;
};

class Scene; // Forward Declaration needed by Mesh

struct Mesh {
	const AssetLoader::MeshData* mesh_data;

	u32       triangle_count;
	Triangle* triangles;

	GLuint vbo;
	GLuint ibo;
	GLuint tbo;
	GLuint tbo_tex;

	Mesh(const AssetLoader::MeshData* mesh_data);

	void init(const Scene& scene, u32 sample_count, const SH_Sample samples[]);

	bool intersects(const Ray& ray) const;

	void render(GLuint uni_tbo_texture) const;
};

class Scene
{
public:
	inline Scene() {
		Mesh mesh = Mesh(AssetLoader::load_mesh(DATA_PATH("box.obj")));
		meshes.emplace_back(mesh);
	}

	inline void init() {
		SH_Sample samples[SAMPLE_COUNT];
		init_samples(samples, SQRT_SAMPLE_COUNT, SH_NUM_BANDS);

		for (u32 i = 0; i < meshes.size(); i++) {
			meshes[i].init(*this, SAMPLE_COUNT, samples);
		}
	}

	inline bool intersects(const Ray& ray) const {
		for (u32 i = 0; i < meshes.size(); i++) {
			if (meshes[i].intersects(ray)) {
				return true;
			}
		}

		return false;
	}

	inline void render(GLuint uni_tbo_texture) {
		for (u32 i = 0; i < meshes.size(); i++) {
			meshes[i].render(uni_tbo_texture);
		}
	}

private:
	Array<Mesh> meshes;
};

