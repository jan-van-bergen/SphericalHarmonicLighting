#include "Scene.h"

#include <fstream>
#include "ScopedTimer.h"

// Forces NVIDIA driver to be used 
extern "C" { _declspec(dllexport) unsigned NvOptimusEnablement = 0x00000001; }

Mesh::Mesh(const char* file_name) : file_name(file_name), mesh_data(AssetLoader::load_mesh(file_name)) {
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

	glm::vec3* positions       = new glm::vec3[vertex_count];
	glm::vec3* transfer_coeffs = new glm::vec3[vertex_count * SH_COEFFICIENT_COUNT];

	for (int i = 0; i < vertex_count; i++) {
		// Copy positions
		positions[i] = mesh_data->vertices[i].position;
	}
	
	u32 length = strlen(file_name);
	char* dat_file_name = new char[length + 1];
	strcpy_s(dat_file_name, length + 1, file_name);

	// Replace .obj with .dat
	dat_file_name[length - 3] = 'd';
	dat_file_name[length - 2] = 'a';
	dat_file_name[length - 1] = 't';

	std::ifstream in_file(dat_file_name, std::ios::in | std::ios::binary); 
	if (in_file.is_open()) {
		u32 file_vertex_count;
		in_file.read(reinterpret_cast<char*>(&file_vertex_count), sizeof(u32));

		if (file_vertex_count != vertex_count) {
			abort();
		}

		in_file.read(reinterpret_cast<char*>(transfer_coeffs), vertex_count * SH_COEFFICIENT_COUNT * sizeof(glm::vec3));
		in_file.close();
	} else {
		Ray ray;
		
		ScopedTimer timer("Mesh");

		 // Iterate over vertices
		for (int i = 0; i < vertex_count; i++) {

			// Initialize SH coefficients to 0
			for (u32 k = 0; k < SH_COEFFICIENT_COUNT; k++) {
				transfer_coeffs[i * SH_COEFFICIENT_COUNT + k] = glm::vec3(0.0f, 0.0f, 0.0f);
			}

			// Iterate over SH samples
			for (u32 j = 0; j < sample_count; j++) {
				float dot = glm::dot(mesh_data->vertices[i].normal, samples[j].direction);

				// Only accept samples within the hemisphere defined by the Vertex normal
				if (dot >= 0.0f) {
					ray.origin    = mesh_data->vertices[i].position + mesh_data->vertices[i].normal * 0.025f;
					ray.direction = samples[j].direction;

					if (!scene.intersects(ray)) {
						for (u32 k = 0; k < SH_COEFFICIENT_COUNT; k++) {
							// Add the contribution of this sample
							transfer_coeffs[i * SH_COEFFICIENT_COUNT + k] += material.diffuse_colour * dot * samples[j].coeffs[k];
						}
					}
				}
			}

			const float normalization_factor = 4.0f * PI / sample_count;

			// Normalize coefficients
			for (u32 k = 0; k < SH_COEFFICIENT_COUNT; k++) {
				transfer_coeffs[i * SH_COEFFICIENT_COUNT + k] *= normalization_factor;
			}

			//printf("Vertex %u out of %u done\n", i, vertex_count);
		}

		std::ofstream out_file(dat_file_name, std::ios::out | std::ios::binary | std::ios::trunc);
		{
			out_file.write(reinterpret_cast<const char*>(&vertex_count), sizeof(u32));
			out_file.write(reinterpret_cast<const char*>(transfer_coeffs), vertex_count * SH_COEFFICIENT_COUNT * sizeof(glm::vec3));
		}
		out_file.close();
	}

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(glm::vec3), positions, GL_STATIC_DRAW);

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(u32), mesh_data->indices, GL_STATIC_DRAW);

	glGenBuffers(1, &tbo);
	glBindBuffer(GL_TEXTURE_BUFFER, tbo);
	glBufferData(GL_TEXTURE_BUFFER, vertex_count * SH_COEFFICIENT_COUNT * sizeof(glm::vec3), transfer_coeffs, GL_STATIC_DRAW);

	glGenTextures(1, &tbo_tex);
	glBindTexture(GL_TEXTURE_BUFFER, tbo_tex);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, tbo);

	delete[] positions;
	delete[] transfer_coeffs;
	
	delete[] dat_file_name;
}

bool Mesh::intersects(const Ray& ray) const {
	// If any Triangle intersects the Ray then the Triangle Mesh intersects the Ray as well
	bool result = false;

	#pragma omp parallel for
	for (int i = 0; i < triangle_count; i++) {
		#pragma omp flush (result)
		if (!result) {
			if (ray.intersects(triangles[i])) {
				result = true;
				#pragma omp flush (result)
			}
		}
	}

	// If none of the Triangles intersect the nthe Triangle Mesh doesn't either
	return result;
}

Triangle* Mesh::closest_triangle(const Ray& ray) const {
	float     min_distance = INFINITY;
	Triangle* closest_triangle = NULL;

	for (int i = 0; i < triangle_count; i++) {
		float distance = ray.distance(triangles[i]);
		if (distance < min_distance) {
			min_distance = distance;
			
			closest_triangle = triangles + i;
		}
	}

	return closest_triangle;
}

void Mesh::render() const {
	// Bind TBO
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_BUFFER, tbo_tex);

	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0); // Position

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glDrawElements(GL_TRIANGLES, triangle_count * 3, GL_UNSIGNED_INT, NULL);

	glDisableVertexAttribArray(0);
}
