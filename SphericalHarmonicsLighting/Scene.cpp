#include "Scene.h"

#include <SDL2/SDL.h>

#include <glm/gtc/matrix_transform.hpp>

#include "VectorMath.h"

#include "SHRotation.h"

#include "ScopedTimer.h"

#include "Util.h"

Scene::Scene() : shader_diffuse(), shader_glossy(), angle(0) {
	mesh_count = 2;
	meshes = ALLOC_ARRAY(Mesh, mesh_count);
	new(&meshes[0]) Mesh(DATA_PATH("Models/MonkeySubdivided2.obj"), shader_glossy);
	new(&meshes[1]) Mesh(DATA_PATH("Models/Plane.obj"),             shader_diffuse);
	
	// @TODO: maybe make the Light a user choice?
	light_count = 1;
	lights = new Light *[light_count];
	lights[0] = new DirectionalLight();
	//lights[0] = new HDRProbeLight(DATA_PATH("Light Probes/grace_probe.float"), 1000);

	camera.position    = glm::vec3(0.0f, 0.0f, 10.0f);
	camera.orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	camera.projection  = glm::perspective(DEG_TO_RAD(45.0f), 1600.0f / 900.0f, 0.1f, 100.0f);
}

void Scene::init() {
	SH::init_rotation();

	SH::Sample* samples = new SH::Sample[SAMPLE_COUNT];
	SH::init_samples(samples, SQRT_SAMPLE_COUNT, SH_NUM_BANDS);

	for (int i = 0; i < mesh_count; i++) {
		meshes[i].init(*this, SAMPLE_COUNT, samples);
	}

	for (int i = 0; i < light_count; i++) {
		lights[i]->init(SAMPLE_COUNT, samples);
	}

	delete[] samples;
}

void Scene::update(float delta, const u8* keys) {
	const float movement_speed = 10.0f;
	const float rotation_speed =  2.0f;

	const glm::vec3& camera_right   = camera.orientation * glm::vec3(1.0f, 0.0f,  0.0f);
	const glm::vec3& camera_up      = camera.orientation * glm::vec3(0.0f, 1.0f,  0.0f);
	const glm::vec3& camera_forward = camera.orientation * glm::vec3(0.0f, 0.0f, -1.0f);

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

	glm::vec3 light_coeffs_rotated[SH_COEFFICIENT_COUNT];
	SH::rotate(glm::angleAxis(angle, glm::vec3(0.0f, 1.0f, 0.0f)), lights[0]->coefficients, light_coeffs_rotated);

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

bool Scene::intersects(const Ray& ray) const {
	for (int i = 0; i < mesh_count; i++) {
		if (meshes[i].intersects(ray)) {
			return true;
		}
	}

	return false;
}