#include "Scene.h"

#include <SDL2/SDL.h>

#include <glm/gtc/matrix_transform.hpp>

#include "VectorMath.h"
#include "SHRotation.h"

#include "ScopedTimer.h"

#include "Util.h"

Scene::Scene() : shader_diffuse(), shader_glossy(), angle(0) {
	mesh_count = 3;
	meshes = ALLOC_ARRAY(Mesh, mesh_count);
	Mesh * monkey = new(&meshes[0]) Mesh(DATA_PATH("Models/MonkeySubdivided2.obj"), shader_diffuse);
	Mesh * plane  = new(&meshes[1]) Mesh(DATA_PATH("Models/Plane.obj"),             shader_diffuse);
	Mesh * test   = new(&meshes[2]) Mesh(DATA_PATH("Models/Test.obj"),              shader_diffuse);
	
	plane->material.albedo = glm::vec3(1.0f, 0.0f, 0.0f);
	test->material.albedo  = glm::vec3(0.0f, 1.0f, 0.0f);

	// @TODO: maybe make the Light a user choice?
	light_count = 1;
	lights = ALLOC_ARRAY(Light *, light_count);
	lights[0] = new DirectionalLight();
	//lights[0] = new HDRProbeLight(DATA_PATH("Light Probes/grace_probe.float"), 1000);

	camera.position    = glm::vec3(0.0f, 0.0f, 10.0f);
	camera.orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	camera.projection  = glm::perspective(DEG_TO_RAD(45.0f), 1600.0f / 900.0f, 0.1f, 100.0f);
}

Scene::~Scene() {
	free(meshes);
	free(lights);
}

void Scene::init() {
	SH::init_rotation();

	SH::Sample* samples = new SH::Sample[SAMPLE_COUNT];
	SH::init_samples(samples);

	bool all_meshes_loaded = true;
	int scene_coeff_count = 0;
	
	glm::vec3 * bounces_scene_coeffs[NUM_BOUNCES + 1];

	for (int m = 0; m < mesh_count; m++) {		
		meshes[m].transfer_coeffs_scene_offset = scene_coeff_count;
		scene_coeff_count += meshes[m].vertex_count * meshes[m].transfer_coeff_count;
	}
	
	bounces_scene_coeffs[0] = new glm::vec3[scene_coeff_count];
	memset(bounces_scene_coeffs[0], 0, scene_coeff_count * sizeof(glm::vec3));

	// Try to load transfer coefficients for all Meshes and record if any Mesh failed to load
	for (int m = 0; m < mesh_count; m++) {
		bool was_loaded = meshes[m].try_to_load_transfer_coeffs(bounces_scene_coeffs[0] + meshes[m].transfer_coeffs_scene_offset);
		all_meshes_loaded &= was_loaded;

		meshes[m].init_material(samples);
	}

	if (!all_meshes_loaded) {	
		printf("No cached transfer coefficients found. These will need to be regenerated by raytracing, this may take a while...\n");

		for (int b = 1; b <= NUM_BOUNCES; b++) {
			bounces_scene_coeffs[b] = new glm::vec3[scene_coeff_count];
			memset(bounces_scene_coeffs[b], 0, scene_coeff_count * sizeof(glm::vec3));
		}

		// First do direct lighting pass
		for (int m = 0; m < mesh_count; m++) {
			meshes[m].init_light_direct(*this, samples, bounces_scene_coeffs[0] + meshes[m].transfer_coeffs_scene_offset);
		}

		// Then do subsequent bounce passes
		for (int b = 1; b <= NUM_BOUNCES; b++) {
			ScopedTimer timer("Bounce");

			for (int m = 0; m < mesh_count; m++) {
				meshes[m].init_light_bounce(*this, samples, bounces_scene_coeffs[b - 1], bounces_scene_coeffs[b] + meshes[m].transfer_coeffs_scene_offset);
			}
		}

		// Sum all bounces of self transferred light back into sh_coeff
		for (int b = 1; b <= NUM_BOUNCES; b++) {
			for (int m = 0; m < mesh_count; m++) {
				glm::vec3       * transfer_coeffs = bounces_scene_coeffs[0] + meshes[m].transfer_coeffs_scene_offset;
				glm::vec3 const * bounce_coeffs   = bounces_scene_coeffs[b] + meshes[m].transfer_coeffs_scene_offset;

				for (int i = 0; i < meshes[m].vertex_count * meshes[m].transfer_coeff_count; i++) {
					transfer_coeffs[i] += bounce_coeffs[i];
				}
			}

			delete[] bounces_scene_coeffs[b];
		}

		// Save transfer coefficients to disk
		for (int m = 0; m < mesh_count; m++) {
			meshes[m].save_transfer_coeffs(bounces_scene_coeffs[0] + meshes[m].transfer_coeffs_scene_offset);
		}

		printf("Transfer coefficients were saved to disk!\n");
	}

	for (int m = 0; m < mesh_count; m++) {
		meshes[m].init_shader(samples, bounces_scene_coeffs[0] + meshes[m].transfer_coeffs_scene_offset);
	}

	delete[] bounces_scene_coeffs[0];

	for (int i = 0; i < light_count; i++) {
		lights[i]->init(samples);
	}

	delete[] samples;
}

void Scene::update(float delta, const u8 * keys) {
	const float movement_speed = 10.0f;
	const float rotation_speed =  2.0f;

	glm::vec3 camera_right   = camera.orientation * glm::vec3(1.0f, 0.0f,  0.0f);
	glm::vec3 camera_up      = camera.orientation * glm::vec3(0.0f, 1.0f,  0.0f);
	glm::vec3 camera_forward = camera.orientation * glm::vec3(0.0f, 0.0f, -1.0f);

	if (keys[SDL_SCANCODE_W]) camera.position += camera_forward * movement_speed * delta;
	if (keys[SDL_SCANCODE_A]) camera.position -= camera_right   * movement_speed * delta;
	if (keys[SDL_SCANCODE_S]) camera.position -= camera_forward * movement_speed * delta;
	if (keys[SDL_SCANCODE_D]) camera.position += camera_right   * movement_speed * delta;

	if (keys[SDL_SCANCODE_LSHIFT]) camera.position -= camera_up * movement_speed * delta;
	if (keys[SDL_SCANCODE_SPACE])  camera.position += camera_up * movement_speed * delta;

	if (keys[SDL_SCANCODE_UP])    camera.orientation = glm::angleAxis(+rotation_speed * delta, camera_right) * camera.orientation;
	if (keys[SDL_SCANCODE_DOWN])  camera.orientation = glm::angleAxis(-rotation_speed * delta, camera_right) * camera.orientation;
	if (keys[SDL_SCANCODE_LEFT])  camera.orientation = glm::angleAxis(+rotation_speed * delta, glm::vec3(0.0f, 1.0f, 0.0f)) * camera.orientation;
	if (keys[SDL_SCANCODE_RIGHT]) camera.orientation = glm::angleAxis(-rotation_speed * delta, glm::vec3(0.0f, 1.0f, 0.0f)) * camera.orientation;

	camera.view_projection = camera.projection * create_view_matrix(camera.position, camera.orientation);

	angle = fmod(angle + delta, 2.0f * PI);

	glm::quat rotation =
		glm::angleAxis(angle,             glm::vec3(0.0f, 1.0f, 0.0f)) * 
		glm::angleAxis(DEG_TO_RAD(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	glm::vec3 light_coeffs_rotated[SH_COEFFICIENT_COUNT];
	SH::rotate(rotation, lights[0]->coefficients, light_coeffs_rotated);

	shader_diffuse.bind();
	shader_diffuse.set_light_coeffs(light_coeffs_rotated);
	shader_diffuse.set_view_projection(camera.view_projection);

	shader_glossy.bind();
	shader_glossy.set_light_coeffs(light_coeffs_rotated);
	shader_glossy.set_view_projection(camera.view_projection);
	shader_glossy.set_camera_position(camera.position);
	shader_glossy.unbind();
}

void Scene::render() const {
	for (int i = 0; i < mesh_count; i++) {
		meshes[i].render();
	}
}

void Scene::debug(GLuint uni_debug_view_projection) const {
	glUniformMatrix4fv(uni_debug_view_projection, 1, GL_FALSE, glm::value_ptr(camera.view_projection));

	for (int i = 0; i < mesh_count; i++) {
		meshes[i].debug();
	}
}

bool Scene::intersects(const Ray & ray) const {
	for (int i = 0; i < mesh_count; i++) {
		if (meshes[i].intersects(ray)) {
			return true;
		}
	}

	return false;
}

float Scene::trace(const Ray & ray, int indices[3], float & u, float & v, const Mesh *& mesh) const {
	float min_distance = INFINITY;

	int   current_indices[3];
	float current_u;
	float current_v;

	for (int m = 0; m < mesh_count; m++) {
		float distance = meshes[m].trace(ray, current_indices, current_u, current_v);
		if (distance < min_distance) {
			min_distance = distance;

			memcpy(indices, current_indices, 3 * sizeof(int));
			u = current_u;
			v = current_v;

			mesh = &meshes[m];
		}
	}

	return min_distance;
}
