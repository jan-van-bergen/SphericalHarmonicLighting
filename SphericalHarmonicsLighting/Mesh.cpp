#include "Scene.h"

#include <fstream>

#include "StringHelper.h"

#include "Util.h"
#include "ScopedTimer.h"

// @TODO: only GLOSSY meshes need normals
struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
};

Mesh::Mesh(const char* file_name, const MeshShader& shader) : file_name(file_name), mesh_data(AssetLoader::load_mesh(file_name)), material(shader) {
	assert(mesh_data->index_count % 3 == 0);
	
	vertex_count = mesh_data->vertex_count;

	triangle_count = mesh_data->index_count / 3;
	triangles      = new Triangle[triangle_count];

	for (int i = 0; i < triangle_count; i++) {
		// Set the three indices of the Triangle
		triangles[i].indices[0] = mesh_data->indices[3*i    ];
		triangles[i].indices[1] = mesh_data->indices[3*i + 1];
		triangles[i].indices[2] = mesh_data->indices[3*i + 2];

		// Set the three Vertex positions of the Triangle, based on the indices
		triangles[i].vertices[0] = mesh_data->vertices[triangles[i].indices[0]].position;
		triangles[i].vertices[1] = mesh_data->vertices[triangles[i].indices[1]].position;
		triangles[i].vertices[2] = mesh_data->vertices[triangles[i].indices[2]].position;

		// Calculate the Plane equation that fits the Triangle
		glm::vec3 edge0 = triangles[i].vertices[1] - triangles[i].vertices[0];
		glm::vec3 edge1 = triangles[i].vertices[2] - triangles[i].vertices[0];

		triangles[i].plane.normal   =  glm::normalize(glm::cross(edge0, edge1));
		triangles[i].plane.distance = -glm::dot(triangles[i].plane.normal, triangles[i].vertices[0]);
	}
	
	{
		ScopedTimer timer("KD Tree Construction");

		Triangle const ** triangles_copy = new Triangle const *[triangle_count];
		{
			for (int i = 0; i < triangle_count; i++) {
				triangles_copy[i] = &triangles[i];
			}

			kd_tree = KD_Node::build(triangle_count, triangles_copy);
		}
		delete[] triangles_copy;

		kd_tree_debugger.init(kd_tree);
	}
}

bool Mesh::try_to_load_transfer_coeffs() {
	transfer_coeffs_file_name = new char[1024];

	u32 file_name_length = strlen(file_name);
	strcpy_s(transfer_coeffs_file_name, file_name_length + 1, file_name);

	int last_dot_index = StringHelper::last_index_of(".", transfer_coeffs_file_name);
	assert(last_dot_index != INVALID);

	transfer_coeff_count = INVALID;
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
	
	transfer_coeffs = new glm::vec3[vertex_count * transfer_coeff_count];
	
	bool was_loaded = false;

	std::ifstream in_file(transfer_coeffs_file_name, std::ios::in | std::ios::binary); 
	if (in_file.is_open()) {
		u32 file_vertex_count;
		in_file.read(reinterpret_cast<char*>(&file_vertex_count), sizeof(u32));

		if (file_vertex_count != vertex_count) {
			return false;
		}

		u32 file_transfer_count;
		in_file.read(reinterpret_cast<char*>(&file_transfer_count), sizeof(u32));

		if (file_transfer_count == transfer_coeff_count) {
			in_file.read(reinterpret_cast<char*>(transfer_coeffs), vertex_count * transfer_coeff_count * sizeof(glm::vec3));

			was_loaded = true;
		}

		in_file.close();
	} 
	
	return was_loaded;
}

void Mesh::save_transfer_coeffs() const {
	assert(transfer_coeffs_file_name);
	assert(transfer_coeffs);

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

void Mesh::init_light_direct(const Scene& scene, const SH::Sample samples[SAMPLE_COUNT]) {
	const int index_count  = mesh_data->index_count;

	ScopedTimer timer("Mesh Direct + Shadowed Lighting");

	Ray ray;
	hits = new bool[vertex_count * SAMPLE_COUNT];
	
	// Iterate over vertices
	for (int v = 0; v < vertex_count; v++) {
		// Initialize SH coefficients to 0
		for (int i = 0; i < transfer_coeff_count; i++) {
			transfer_coeffs[v * transfer_coeff_count + i] = glm::vec3(0.0f, 0.0f, 0.0f);
		}

		// Iterate over SH samples
		for (int s = 0; s < SAMPLE_COUNT; s++) {
			float dot = glm::dot(mesh_data->vertices[v].normal, samples[s].direction);

			// Only accept samples within the hemisphere defined by the Vertex normal
			if (dot >= 0.0f) {
				ray.origin    = mesh_data->vertices[v].position + mesh_data->vertices[v].normal * EPSILON;
				ray.direction = samples[s].direction;

				bool hit = scene.intersects(ray);
				hits[v * SAMPLE_COUNT + s] = hit;

				// If the Ray was not occluded
				if (!hit) {
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
			} else {
				hits[v * SAMPLE_COUNT + s] = false;
			}
		}

		const float normalization_factor = 4.0f * PI / SAMPLE_COUNT;

		// Normalize coefficients
		for (int i = 0; i < transfer_coeff_count; i++) {
			transfer_coeffs[v * transfer_coeff_count + i] *= normalization_factor;
		}

		//printf("Bounce 0: Vertex %u out of %u done\n", v, vertex_count);
	}
}

void Mesh::init_light_bounce(const Scene& scene, const SH::Sample samples[SAMPLE_COUNT], const glm::vec3 previous_bounce_transfer_coeffs[], glm::vec3 bounce_transfer_coeffs[]) {
	Ray ray;
	
	int indices[3];
	float weight_u;
	float weight_v;
	glm::vec3 albedo;

	// Iterate over vertices
	for (int v = 0; v < vertex_count; v++) {
		// Iterate over samples
		for (int s = 0; s < SAMPLE_COUNT; s++) {
			// If the ray in the current sample direction hit anything in the direct lighting pass
			if (hits[v * SAMPLE_COUNT + s]) {
				float dot = glm::dot(samples[s].direction, mesh_data->vertices[v].normal);
				// if ray inside hemisphere, continue processing.
				if (dot > 0.0f) {
					ray.origin    = mesh_data->vertices[v].position + mesh_data->vertices[v].normal * EPSILON;
					ray.direction = samples[s].direction;

					float distance = scene.trace(ray, indices, weight_u, weight_v, albedo);	
					assert(distance != INFINITY);
					
					float weight_w = 1.0f - (weight_u + weight_v);

					const glm::vec3 * transfer_coeffs_vertex0 = previous_bounce_transfer_coeffs + indices[0];
					const glm::vec3 * transfer_coeffs_vertex1 = previous_bounce_transfer_coeffs + indices[1];
					const glm::vec3 * transfer_coeffs_vertex2 = previous_bounce_transfer_coeffs + indices[2];

					// Sum reflected SH light for this vertex
					// Lerp hit vertices SH vectors to get SH at hit point
					for (int k = 0; k < transfer_coeff_count; k++) {
						bounce_transfer_coeffs[v * transfer_coeff_count + k] += albedo * dot * (
							weight_u * transfer_coeffs_vertex0[k] + 
							weight_v * transfer_coeffs_vertex1[k] + 
							weight_w * transfer_coeffs_vertex2[k]
						);
					}
				}
			}
		}
		
		//printf("Bounce n: Vertex %u out of %u done\n", i, vertex_count);
	}
	
	const float normalization_factor = 4.0f * PI / SAMPLE_COUNT;

	for (int j = 0; j < vertex_count * transfer_coeff_count; j++) {
		bounce_transfer_coeffs[j] *= normalization_factor;
	}
}

void Mesh::init_shader(const SH::Sample samples[SAMPLE_COUNT]) {
	Vertex * vertices = new Vertex[vertex_count];
	
	// Copy positions and normals
	for (int i = 0; i < vertex_count; i++) {
		vertices[i].position = mesh_data->vertices[i].position;
		vertices[i].normal   = mesh_data->vertices[i].normal;
	}

	// Bind the vertices
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(Vertex), vertices, GL_STATIC_DRAW);

	// Bind the indices
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh_data->index_count * sizeof(u32), mesh_data->indices, GL_STATIC_DRAW);

	// Bind the transfer coefficients to a Texture Buffer Object (TBO)
	glGenBuffers(1, &tbo);
	glBindBuffer(GL_TEXTURE_BUFFER, tbo);
	glBufferData(GL_TEXTURE_BUFFER, vertex_count * transfer_coeff_count * sizeof(glm::vec3), transfer_coeffs, GL_STATIC_DRAW);

	// Attach the TBO to the TBO texture
	glGenTextures(1, &tbo_tex);
	glBindTexture(GL_TEXTURE_BUFFER, tbo_tex);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, tbo);

	// Afer uploading this data to the GPU it can be removed from CPU RAM
	delete[] vertices;
	delete[] transfer_coeffs;

	// For GLOSSY materials, we need to convolve with a BRDF (Phong lobe)
	// We compute this here and pass it to the Shader
	if (material.shader.type == MeshShader::Type::GLOSSY) {	
		// Specular lobe function
		// NOTE: This function depends only on theta, allowing for fast convolution with this kernel
		SH::PolarFunction specular_lobe = [](float theta, float phi) {
			const float spec = 1.0f;

			float dot = cos(theta);

			if (dot > 0.0f) return glm::vec3(pow(dot, spec));
		
			return glm::vec3(0.0f);
		};

		// Project the BRDF into Spherical Harmonic representation using Monte Carlo integration
		glm::vec3 brdf_coeffs_full[SH_COEFFICIENT_COUNT];
		for (int i = 0; i < SH_COEFFICIENT_COUNT; i++) {
			brdf_coeffs_full[i] = glm::vec3(0.0f, 0.0f, 0.0f);
		}
		SH::project_polar_function(specular_lobe, SAMPLE_COUNT, samples, brdf_coeffs_full);

		// Because the kernel is only dependend on theta, we can condense it into a representation with only 
		// SH_NUM_BANDS coefficients instead of the normal SH_NUM_BANDS x SH_NUM_BANDS coefficients.
		for (int l = 0; l < SH_NUM_BANDS; l++) {
			material.brdf_coeffs[l] = sqrt(4.0f * PI / (2.0f * l + 1.0f)) * brdf_coeffs_full[l*(l + 1)];
		}
	}
}

bool Mesh::intersects(const Ray& ray) const {
	return kd_tree->intersects(ray);
}

float Mesh::trace(const Ray& ray, int indices[3], float& u, float& v) const {
	return kd_tree->trace(ray, indices, u, v);
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

void Mesh::debug() const {
	kd_tree_debugger.draw();
}