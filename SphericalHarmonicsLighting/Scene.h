#pragma once
#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "AssetLoader.h"

#include "Ray.h"
#include "SphericalSamples.h"

#include "Types.h"
#include "Util.h"

#include "VectorMath.h"

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

struct Camera {
	glm::vec3 position;
	glm::quat orientation;

	glm::mat4 projection;
	glm::mat4 view_projection;
};

class Scene
{
public:
	inline Scene() {
		Mesh mesh = Mesh(AssetLoader::load_mesh(DATA_PATH("box.obj")));
		meshes.emplace_back(mesh);

		camera = Camera();
		camera.position    = glm::vec3(0.0f, 0.0f, 10.0f);
		camera.orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		camera.projection  = glm::perspectiveFov(RAD_TO_DEG(110.0f), 1600.0f, 900.0f, 0.1f, 100.0f); // @HARDCODED
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

	inline void render(GLuint uni_tbo_texture, GLuint uni_view_projection) {
		camera.view_projection = camera.projection * create_view_matrix(camera.position, camera.orientation);

		glUniformMatrix4fv(uni_view_projection, 1, GL_FALSE, glm::value_ptr(camera.view_projection));

		for (u32 i = 0; i < meshes.size(); i++) {
			meshes[i].render(uni_tbo_texture);
		}
	}

private:
	Array<Mesh> meshes;

	Camera camera;
};

