#include "Scene.h"

Mesh::Mesh(const AssetLoader::MeshData* mesh_data) : mesh_data(mesh_data) {
	assert(mesh_data->index_count % 3 == 0);

	triangle_count = mesh_data->index_count / 3;
	triangles      = new Triangle[triangle_count];

	for (u32 i = 0; i < triangle_count; i++) {
		// Set the three Vertex positions of the Triangle
		triangles[i].vertices[0] = mesh_data->vertices[mesh_data->indices[3*i    ]].position;
		triangles[i].vertices[1] = mesh_data->vertices[mesh_data->indices[3*i + 1]].position;
		triangles[i].vertices[2] = mesh_data->vertices[mesh_data->indices[3*i + 2]].position;

		// Calculate the Plane equation that fits the Triangle
		glm::vec3 edge0 = triangles[i].vertices[1] - triangles[i].vertices[0];
		glm::vec3 edge1 = triangles[i].vertices[2] - triangles[i].vertices[0];

		triangles[i].plane.normal   =  glm::normalize(glm::cross(edge0, edge1));
		triangles[i].plane.distance = -glm::dot(triangles[i].plane.normal, triangles[i].vertices[0]);
	}
}

void Mesh::init(const Scene& scene, u32 sample_count, const SH_Sample samples[]) {
	const u32 vertex_count = mesh_data->vertex_count;
	const u32 index_count  = mesh_data->index_count;

	glm::vec3* positions = new glm::vec3[vertex_count];
	float*     coeffs    = new float    [vertex_count * SH_COEFFICIENT_COUNT];

	// Iterate over vertices
	for (u32 i = 0; i < vertex_count; i++) {
		// Copy positions
		positions[i] = mesh_data->vertices[i].position;

		// Initialize SH coefficients to 0
		for (u32 k = 0; k < SH_COEFFICIENT_COUNT; k++) {
			coeffs[i * SH_COEFFICIENT_COUNT + k] = 0.0f;
		}

		// Iterate over SH samples
		for (u32 j = 0; j < sample_count; j++) {
			float dot = glm::dot(mesh_data->vertices[i].normal, samples[j].direction);

			// Only accept samples within the hemisphere defined by the Vertex normal
			if (dot >= 0.0f) {
				Ray ray;
				ray.origin    = mesh_data->vertices[i].position + mesh_data->vertices[i].normal * 0.001f;
				ray.direction = samples[j].direction;

				//if (scene.intersects(ray)) {
				for (u32 k = 0; k < SH_COEFFICIENT_COUNT; k++) {
					// Add the contribution of this sample
					coeffs[i * SH_COEFFICIENT_COUNT + k] += dot * samples[j].coeffs[k];
				}
				//}
			}
		}

		const float normalization_factor = 4.0f * PI / sample_count;

		// Normalize coefficients
		for (u32 k = 0; k < SH_COEFFICIENT_COUNT; k++) {
			coeffs[i * SH_COEFFICIENT_COUNT + k] *= normalization_factor;
		}
	}

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertex_count, positions, GL_STATIC_DRAW);

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * index_count, mesh_data->indices, GL_STATIC_DRAW);

	glGenBuffers(1, &tbo);
	glBindBuffer(GL_TEXTURE_BUFFER, tbo);
	glBufferData(GL_TEXTURE_BUFFER, sizeof(coeffs), coeffs, GL_STATIC_DRAW);
	glGenTextures(1, &tbo_tex);

	delete[] positions;
	delete[] coeffs;
}

bool Mesh::intersects(const Ray& ray) const {
	// If any Triangle intersects the Ray then the Triangle Mesh intersects the Ray as well
	for (u32 i = 0; i < triangle_count; i++) {
		if (triangles[i].intersects(ray)) {
			return true;
		}
	}

	// If none of the Triangles intersect the nthe Triangle Mesh doesn't either
	return false;
}

void Mesh::render(GLuint uni_tbo_texture) const {
	// Bind TBO
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_BUFFER, tbo_tex);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, tbo); // @TODO: check if this is required for every frame?
	glUniform1i(uni_tbo_texture, 0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0); // Position

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glDrawElements(GL_TRIANGLES, triangle_count * 3, GL_UNSIGNED_INT, NULL);

}
