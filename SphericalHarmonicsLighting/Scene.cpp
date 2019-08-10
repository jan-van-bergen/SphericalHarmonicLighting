#include "Scene.h"

#include <SDL2/SDL.h>

Scene::Scene() {
	Mesh monkey = Mesh(DATA_PATH("MonkeySubdivided2.obj"));
	Mesh plane  = Mesh(DATA_PATH("Plane.obj"));

	meshes.emplace_back(monkey);
	meshes.emplace_back(plane);

	camera = Camera();
	camera.position    = glm::vec3(0.0f, 0.0f, 10.0f);
	camera.orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	camera.projection  = glm::perspective(DEG_TO_RAD(45.0f), 1600.0f / 900.0f, 0.1f, 100.0f);
	//camera.projection  = glm::perspectiveFov(RAD_TO_DEG(110.0f), 1600.0f, 900.0f, 0.1f, 100.0f); // @HARDCODED
}

void Scene::init() {
	SH_Sample* samples = new SH_Sample[SAMPLE_COUNT];
	init_samples(samples, SQRT_SAMPLE_COUNT, SH_NUM_BANDS);

	for (u32 i = 0; i < meshes.size(); i++) {
		meshes[i].init(*this, SAMPLE_COUNT, samples);
	}

	// Project the light function in polar coordinates to obtain the light's SH coeffs
	auto light_function = [](float theta, float phi) {
		return (theta < PI / 6.0f) ? 1.0f : 0.0f;
	};

	project_polar_function(light_function, SAMPLE_COUNT, samples, light_coeffs);

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
}

void Scene::render(GLuint uni_view_projection, GLuint uni_light_coeffs) const {
	glUniform1fv(uni_light_coeffs, SH_COEFFICIENT_COUNT, light_coeffs);

	glUniformMatrix4fv(uni_view_projection, 1, GL_FALSE, glm::value_ptr(camera.view_projection));

	for (u32 i = 0; i < meshes.size(); i++) {
		meshes[i].render();
	}
}

bool Scene::intersects(const Ray& ray) const {
	for (u32 i = 0; i < meshes.size(); i++) {
		if (meshes[i].intersects(ray)) {
			return true;
		}
	}

	return false;
}