#pragma once
#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "AssetLoader.h"

#include "Ray.h"
#include "KDTree.h"

#include "Light.h"

#include "MeshShaders.h"

#define NUM_BOUNCES 3

struct Material {
	const MeshShader& shader;

	float     specular_power = 1.0f;
	glm::vec3 albedo         = glm::vec3(1.0f, 1.0f, 1.0f);

	glm::vec3 brdf_coeffs[SH_NUM_BANDS]; // NOTE: only used by GLOSSY shader

	inline Material(const MeshShader& shader) : shader(shader) { };
};

class Scene; // Forward Declaration needed by Mesh

struct Mesh {
private:
	const char * file_name;
	const AssetLoader::MeshData * mesh_data;
	
	char * transfer_coeffs_file_name;

	KD_Node *        kd_tree;
	KD_Node_Debugger kd_tree_debugger;
	
	GLuint vbo;
	GLuint ibo;
	GLuint tbo;
	GLuint tbo_tex;

	bool * hits; // @TODO: OPTIMIZE!!!

public:
	int        triangle_count;
	Triangle * triangles;
	
	int vertex_count;
	int transfer_coeff_count; // Either SH_COEFFICIENT_COUNT or SH_COEFFICIENT_COUNT^2, depending on DIFFUSE / GLOSSY Shader
	int transfer_coeffs_scene_offset;

	Material material;

	Mesh(const char* file_name, const MeshShader& shader);

	bool try_to_load_transfer_coeffs(glm::vec3 transfer_coeffs[]);
	void        save_transfer_coeffs(glm::vec3 transfer_coeffs[]) const;

	void init_material(const SH::Sample[SAMPLE_COUNT]);
	void init_light_direct(const Scene& scene, const SH::Sample[SAMPLE_COUNT], glm::vec3 transfer_coeffs[]);
	void init_light_bounce(const Scene& scene, const SH::Sample[SAMPLE_COUNT], const glm::vec3 previous_bounce_transfer_coeffs[], glm::vec3 bounce_transfer_coeffs[]);
	void init_shader(const SH::Sample[SAMPLE_COUNT], glm::vec3 transfer_coeffs[]);

	bool  intersects(const Ray& ray) const;
	float trace     (const Ray& ray, int indices[3], float& u, float& v) const;

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
	~Scene();

	void init();

	void update(float delta, const u8 * keys);

	void render() const;

	void debug(GLuint uni_debug_view_projection) const;

	bool  intersects(const Ray& ray) const;
	float trace     (const Ray& ray, int indices[3], float& u, float& v, const Mesh *& mesh) const;

private:
	const DiffuseShader shader_diffuse;
	const GlossyShader  shader_glossy;

	Mesh * meshes;
	int    mesh_count;

	Light ** lights;
	int      light_count;

	Camera camera;

	float angle;
};
