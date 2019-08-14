#pragma once
#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "AssetLoader.h"

#include "Ray.h"
#include "KDTree.h"
#include "SphericalSamples.h"

#include "Light.h"

#include "Types.h"
#include "Util.h"

#include "VectorMath.h"

#define SQRT_SAMPLE_COUNT 50u
#define SAMPLE_COUNT      (SQRT_SAMPLE_COUNT * SQRT_SAMPLE_COUNT)

struct Material {
	glm::vec3 diffuse_colour = glm::vec3(1.0f, 1.0f, 1.0f);
};

class Scene; // Forward Declaration needed by Mesh

struct Mesh {
private:
	const char* file_name;
	const AssetLoader::MeshData* mesh_data;
	
	GLuint vbo;
	GLuint ibo;
	GLuint tbo;
	GLuint tbo_tex;

public:
	u32       triangle_count;
	Triangle* triangles;

	Material material;

	Mesh(const char* file_name);

	void init(const Scene& scene, u32 sample_count, const SH_Sample samples[]);
	
	bool      intersects      (const Ray& ray) const;
	Triangle* closest_triangle(const Ray& ray) const;

	void render() const;
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
	~Scene();

	void init();

	void update(float delta, const u8* keys);

	void render(GLuint uni_view_projection, GLuint uni_light_coeffs) const;

	bool intersects(const Ray& ray) const;

private:
	Array<Mesh>   meshes;
	Array<Light*> lights;

	KD_Node * kd_tree;

	Camera camera;

	float angle;
};

