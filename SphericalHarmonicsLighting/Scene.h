#pragma once
#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "AssetLoader.h"

#include "Ray.h"
#include "KDTree.h"

#include "Light.h"

#include "MeshShaders.h"

#define SQRT_SAMPLE_COUNT 50
#define SAMPLE_COUNT (SQRT_SAMPLE_COUNT * SQRT_SAMPLE_COUNT)

struct Material {
	const MeshShader& shader;

	glm::vec3 diffuse_colour = glm::vec3(1.0f, 1.0f, 1.0f);

	glm::vec3 brdf_coeffs[SH_NUM_BANDS]; // NOTE: only used by GLOSSY shader

	inline Material(const MeshShader& shader) : shader(shader) { };
};

class Scene; // Forward Declaration needed by Mesh

struct Mesh {
private:
	const char* file_name;
	const AssetLoader::MeshData* mesh_data;
	
	KD_Node *        kd_tree;
	KD_Node_Debugger kd_tree_debugger;

	GLuint vbo;
	GLuint ibo;
	GLuint tbo;
	GLuint tbo_tex;

public:
	int        triangle_count;
	Triangle * triangles;

	Material material;

	Mesh(const char* file_name, const MeshShader& shader);

	void init(const Scene& scene, int sample_count, const SH::Sample samples[]);
	
	bool       intersects      (const Ray& ray) const;
	Triangle * closest_triangle(const Ray& ray) const;

	void render() const;

	void debug() const;
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

	void render() const;

	void debug(GLuint uni_debug_view_projection) const;

	bool intersects(const Ray& ray) const;

private:
	const DiffuseShader shader_diffuse;
	const GlossyShader  shader_glossy;

	Array<Mesh>   meshes;
	Array<Light*> lights;

	Camera camera;

	float angle;
};
