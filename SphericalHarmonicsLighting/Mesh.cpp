#include "Scene.h"

#include <fstream>

#include "String.h"

#include "Util.h"
#include "ScopedTimer.h"

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
};

Mesh::Mesh(const char* file_name, const MeshShader& shader) : file_name(file_name), mesh_data(AssetLoader::load_mesh(file_name)), material(shader) {
	assert(mesh_data->index_count % 3 == 0);

	triangle_count = mesh_data->index_count / 3;
	triangles      = new Triangle[triangle_count];

	for (int i = 0; i < triangle_count; i++) {
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

void Mesh::init(const Scene& scene, int sample_count, const SH::Sample samples[]) {
	const int vertex_count = mesh_data->vertex_count;
	const int index_count  = mesh_data->index_count;

	Vertex * vertices = new Vertex[vertex_count];

	char transfer_coeffs_file_name[1024];

	u32 file_name_length = strlen(file_name);
	strcpy_s(transfer_coeffs_file_name, file_name_length + 1, file_name);

	int last_dot_index = last_index_of(".", transfer_coeffs_file_name);
	assert(last_dot_index != INVALID);

	int transfer_coeff_count = INVALID;
	switch (material.shader.type) {
		case MeshShader::Type::DIFFUSE: {
			// For diffuse transfer functions we use a SH_COEFFICIENT_COUNT dimensional vector
			transfer_coeff_count = SH_COEFFICIENT_COUNT;

			const char * diffuse_str = "_diffuse.dat";
			strcpy_s(transfer_coeffs_file_name + last_dot_index, strlen(diffuse_str) + 1, diffuse_str);
		} break;

		case MeshShader::Type::GLOSSY: {
			// For glossy transfer functions we use a SH_COEFFICIENT_COUNT x SH_COEFFICIENT_COUNT matrix
			transfer_coeff_count = SH_COEFFICIENT_COUNT * SH_COEFFICIENT_COUNT;

			const char * glossy_str = "_glossy.dat";
			strcpy_s(transfer_coeffs_file_name + last_dot_index, strlen(glossy_str) + 1, glossy_str);
		} break;

		default: abort();
	}
	glm::vec3 * transfer_coeffs = new glm::vec3[vertex_count * transfer_coeff_count];

	// Copy positions and normals
	for (int i = 0; i < vertex_count; i++) {
		vertices[i].position = mesh_data->vertices[i].position;
		vertices[i].normal   = mesh_data->vertices[i].normal;
	}

	bool generate = true;
	
	std::ifstream in_file(transfer_coeffs_file_name, std::ios::in | std::ios::binary); 
	if (in_file.is_open()) {
		u32 file_vertex_count;
		in_file.read(reinterpret_cast<char*>(&file_vertex_count), sizeof(u32));

		if (file_vertex_count != vertex_count) {
			abort();
		}

		u32 file_transfer_count;
		in_file.read(reinterpret_cast<char*>(&file_transfer_count), sizeof(u32));

		if (file_transfer_count == transfer_coeff_count) {
			generate = false;

			in_file.read(reinterpret_cast<char*>(transfer_coeffs), vertex_count * transfer_coeff_count * sizeof(glm::vec3));
		}

		in_file.close();
	} 
	
	if (generate) {
		Ray ray;
		
		ScopedTimer timer("Mesh");

		 // Iterate over vertices
		for (int v = 0; v < vertex_count; v++) {
			// Initialize SH coefficients to 0
			for (int i = 0; i < transfer_coeff_count; i++) {
				transfer_coeffs[v * transfer_coeff_count + i] = glm::vec3(0.0f, 0.0f, 0.0f);
			}

			// Iterate over SH samples
			for (int s = 0; s < sample_count; s++) {
				float dot = glm::dot(mesh_data->vertices[v].normal, samples[s].direction);

				// Only accept samples within the hemisphere defined by the Vertex normal
				if (dot >= 0.0f) {
					ray.origin    = mesh_data->vertices[v].position + mesh_data->vertices[v].normal * 0.025f;
					ray.direction = samples[s].direction;

					if (!scene.intersects(ray)) {
						switch (material.shader.type) {
							// For diffuse materials, compose the transfer vector.
							// This vector includes the BDRF, incorporating the albedo colour, a lambertian diffuse factor (dot) and a SH sample
							case MeshShader::Type::DIFFUSE: {
								for (int i = 0; i < SH_COEFFICIENT_COUNT; i++) {
									// Add the contribution of this sample
									transfer_coeffs[v * SH_COEFFICIENT_COUNT + i] += material.diffuse_colour * dot * samples[s].coeffs[i];
								}
							} break;

							// For glossy materials, compose the transfer matrix.
							// This matrix does not include the BDRF, incorporating only two SH samples
							case MeshShader::Type::GLOSSY: {
								for (int j = 0; j < SH_COEFFICIENT_COUNT; j++) {
									for (int i = 0; i < SH_COEFFICIENT_COUNT; i++) {
										// Add the contribution of this sample
										transfer_coeffs[(v * SH_COEFFICIENT_COUNT + j) * SH_COEFFICIENT_COUNT + i] += samples[s].coeffs[j] * samples[s].coeffs[i];
									}
								}
							} break;
						}
					}
				}
			}

			const float normalization_factor = 4.0f * PI / sample_count;

			// Normalize coefficients
			for (int i = 0; i < transfer_coeff_count; i++) {
				transfer_coeffs[v * transfer_coeff_count + i] *= normalization_factor;
			}

			printf("Vertex %u out of %u done\n", v, vertex_count);
		}

		// Save the coefficients to a file so that they can be reloaded at a later time
		std::ofstream out_file(transfer_coeffs_file_name, std::ios::out | std::ios::binary | std::ios::trunc);
		{
			// Write vertx count
			out_file.write(reinterpret_cast<const char*>(&vertex_count), sizeof(u32));
			// Write transfer coefficient count
			out_file.write(reinterpret_cast<const char*>(&transfer_coeff_count), sizeof(u32));
			// Write the actual data, either the transfer vector or the transfer matrix
			out_file.write(reinterpret_cast<const char*>(transfer_coeffs), vertex_count * transfer_coeff_count * sizeof(glm::vec3));
		}
		out_file.close();
	}

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(Vertex), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(u32), mesh_data->indices, GL_STATIC_DRAW);

	glGenBuffers(1, &tbo);
	glBindBuffer(GL_TEXTURE_BUFFER, tbo);
	glBufferData(GL_TEXTURE_BUFFER, vertex_count * transfer_coeff_count * sizeof(glm::vec3), transfer_coeffs, GL_STATIC_DRAW);

	glGenTextures(1, &tbo_tex);
	glBindTexture(GL_TEXTURE_BUFFER, tbo_tex);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, tbo);

	delete[] vertices;
	delete[] transfer_coeffs;

	if (material.shader.type == MeshShader::Type::GLOSSY) {	
		SH::PolarFunction func = [](float theta, float phi) {
			const float spec = 1.0f;

			float dot = cos(theta);

			if (dot >= 0.0f) return glm::vec3(pow(dot, spec));
		
			return glm::vec3(0.0f);
		};

		glm::vec3 brdf_coeffs_full[SH_COEFFICIENT_COUNT];
		for (int i = 0; i < SH_COEFFICIENT_COUNT; i++) {
			brdf_coeffs_full[i] = glm::vec3(0.0f, 0.0f, 0.0f);
		}
		SH::project_polar_function(func, SAMPLE_COUNT, samples, brdf_coeffs_full);

		for (int l = 0; l < SH_NUM_BANDS; l++) {
			material.brdf_coeffs[l] = sqrt(4.0f * PI / (2.0f * l + 1.0f)) * brdf_coeffs_full[l*(l + 1)];
		}
	}
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

Triangle * Mesh::closest_triangle(const Ray& ray) const {
	float      min_distance = INFINITY;
	Triangle * closest_triangle = NULL;

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
	material.shader.bind();

	if (material.shader.type == MeshShader::Type::GLOSSY) {	
		static_cast<const GlossyShader&>(material.shader).set_brdf_coeffs(material.brdf_coeffs);
	}

	// Bind TBO
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_BUFFER, tbo_tex);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),                 0); // Position
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12); // Normal

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glDrawElements(GL_TRIANGLES, triangle_count * 3, GL_UNSIGNED_INT, NULL);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}
