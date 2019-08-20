#include "Scene.h"

#include <SDL2/SDL.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "VectorMath.h"

#include "SHRotation.h"
#include "SphericalSamples.h"

#include "ScopedTimer.h"

#include "Util.h"

Scene::Scene() : angle(0) {
	Mesh monkey = Mesh(DATA_PATH("Models/MonkeySubdivided2.obj"));
	Mesh plane  = Mesh(DATA_PATH("Models/Plane.obj"));

	meshes.emplace_back(monkey);
	meshes.emplace_back(plane);

	lights.emplace_back(new DirectionalLight());
	//lights.emplace_back(new HDRProbeLight(DATA_PATH("Light Probes/grace_probe.float"), 1000));

	camera = Camera();
	camera.position    = glm::vec3(0.0f, 0.0f, 10.0f);
	camera.orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	camera.projection  = glm::perspective(DEG_TO_RAD(45.0f), 1600.0f / 900.0f, 0.1f, 100.0f);
	//camera.projection  = glm::perspectiveFov(RAD_TO_DEG(110.0f), 1600.0f, 900.0f, 0.1f, 100.0f); // @HARDCODED

	kd_tree = NULL;
}

Scene::~Scene() {
	if (kd_tree) delete kd_tree;
}

void Scene::init() {
	sh_init_rotation();

	SH_Sample* samples = new SH_Sample[SAMPLE_COUNT];
	init_samples(samples, SQRT_SAMPLE_COUNT, SH_NUM_BANDS);

	{
		ScopedTimer timer("KD Tree Construction");

		int total_triangle_count = 0;
		for (int i = 0; i < meshes.size(); i++) {
			total_triangle_count += meshes[i].triangle_count;
		}

		Triangle const ** total_triangles = new Triangle const *[total_triangle_count];

		int offset = 0;
		for (int i = 0; i < meshes.size(); i++) {
			for (int j = 0; j < meshes[i].triangle_count; j++) {
				total_triangles[offset + j] = meshes[i].triangles + j;
			}

			offset += meshes[i].triangle_count;
		}

		kd_tree = KD_Node::build(total_triangle_count, total_triangles);

		delete[] total_triangles;
	}

	kd_tree_debugger.init(kd_tree);

	for (int i = 0; i < meshes.size(); i++) {
		meshes[i].init(*this, SAMPLE_COUNT, samples);
	}

	for (int i = 0; i < lights.size(); i++) {
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

	sh_rotate(glm::angleAxis(angle, glm::vec3(0.0f, 1.0f, 0.0f)), lights[0]->coefficients, light_coeffs_rotated);

	glUniform3fv(uni_light_coeffs, SH_COEFFICIENT_COUNT, reinterpret_cast<const GLfloat*>(light_coeffs_rotated));

	glUniformMatrix4fv(uni_view_projection, 1, GL_FALSE, glm::value_ptr(camera.view_projection));

	for (int i = 0; i < meshes.size(); i++) {
		meshes[i].render();
	}
}

void Scene::debug(GLuint uni_debug_view_projection) const {
	glUniformMatrix4fv(uni_debug_view_projection, 1, GL_FALSE, glm::value_ptr(camera.view_projection));

	kd_tree_debugger.draw();
}

bool Scene::intersects(const Ray& ray) const {
	return kd_tree->intersects(ray);
}