#include "Scene.h"

#include <SDL2/SDL.h>

#include "SHRotation.h"

Scene::Scene() {
	Mesh monkey = Mesh(DATA_PATH("MonkeySubdivided1.obj"));
	Mesh plane  = Mesh(DATA_PATH("Plane.obj"));

	meshes.emplace_back(monkey);
	meshes.emplace_back(plane);

	lights.emplace_back(new DirectionalLight());
	//lights.emplace_back(new HDRProbeLight(DATA_PATH("grace_probe.float"), 1000));

	camera = Camera();
	camera.position    = glm::vec3(0.0f, 0.0f, 10.0f);
	camera.orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	camera.projection  = glm::perspective(DEG_TO_RAD(45.0f), 1600.0f / 900.0f, 0.1f, 100.0f);
	//camera.projection  = glm::perspectiveFov(RAD_TO_DEG(110.0f), 1600.0f, 900.0f, 0.1f, 100.0f); // @HARDCODED
}

void Scene::init() {
	SH_Sample* samples = new SH_Sample[SAMPLE_COUNT];
	init_samples(samples, SQRT_SAMPLE_COUNT, SH_NUM_BANDS);

	u32 total_triangle_count = 0;
	for (u32 i = 0; i < meshes.size(); i++) {
		total_triangle_count += meshes[i].triangle_count;
	}

	Triangle const ** total_triangles = new Triangle const *[total_triangle_count];

	u32 offset = 0;
	for (u32 i = 0; i < meshes.size(); i++) {
		for (u32 j = 0; j < meshes[i].triangle_count; j++) {
			total_triangles[offset + j] = meshes[i].triangles + j;
		}

		offset += meshes[i].triangle_count;
	}

	kd_tree = KD_Node::build(total_triangle_count, total_triangles);

	delete[] total_triangles;

	for (u32 i = 0; i < meshes.size(); i++) {
		meshes[i].init(*this, SAMPLE_COUNT, samples);
	}

	for (u32 i = 0; i < lights.size(); i++) {
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
}

void Scene::render(GLuint uni_view_projection, GLuint uni_light_coeffs) const {
	glm::vec3 light_coeffs_rotated[SH_COEFFICIENT_COUNT];

	rotate(glm::angleAxis(angle, glm::vec3(0.0f, 1.0f, 0.0f)), lights[0]->coefficients, light_coeffs_rotated);

	glUniform3fv(uni_light_coeffs, SH_COEFFICIENT_COUNT, reinterpret_cast<const GLfloat*>(light_coeffs_rotated));

	glUniformMatrix4fv(uni_view_projection, 1, GL_FALSE, glm::value_ptr(camera.view_projection));

	for (u32 i = 0; i < meshes.size(); i++) {
		meshes[i].render();
	}
}

bool Scene::intersects(const Ray& ray) const {
	return kd_tree->intersects(ray);
}