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
	Scene();

	void init();

	void update(float delta, const u8* keys);

	void render(GLuint uni_tbo_texture, GLuint uni_view_projection) const;

	bool intersects(const Ray& ray) const;

private:
	Array<Mesh> meshes;

	Camera camera;
};

