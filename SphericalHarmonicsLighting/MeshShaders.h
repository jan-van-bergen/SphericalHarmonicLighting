#pragma once
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "SphericalHarmonics.h"

// Base class that provides the abstraction to differentiate between Diffuse and Glossy materials
class MeshShader : public Shader {
public:
	const enum Type { DIFFUSE, GLOSSY } type;

	inline MeshShader(
		Type type,
		const char* vertex_filename, 
		const char* fragment_filename, 
		const char* geometry_filename = nullptr, 
		const Defines& defines = Defines(0, nullptr, nullptr)) :
		Shader(vertex_filename, fragment_filename, geometry_filename, defines),

		type(type),

		uni_tbo_texture    (get_uniform("tbo_texture")),
		uni_light_coeffs   (get_uniform("light_coeffs")),
		uni_view_projection(get_uniform("view_projection"))
		{
			bind();
			{
				glUniform1i(uni_tbo_texture, 0);
			}
			unbind();
		};

	inline void set_light_coeffs(const glm::vec3 light_coeffs[]) const {
		glUniform3fv(uni_light_coeffs, SH_COEFFICIENT_COUNT, reinterpret_cast<const GLfloat*>(light_coeffs));
	}

	inline void set_view_projection(const glm::mat4& view_projection) const {
		glUniformMatrix4fv(uni_view_projection, 1, GL_FALSE, glm::value_ptr(view_projection));
	}

protected:
	const GLuint uni_tbo_texture;
	const GLuint uni_light_coeffs;
	const GLuint uni_view_projection;
};

class DiffuseShader : public MeshShader {
public:
	DiffuseShader();
};

class GlossyShader : public MeshShader {
public:
	GlossyShader();

	inline void set_brdf_coeffs(const glm::vec3 coeffs[SH_NUM_BANDS]) const {
		glUniform3fv(uni_brdf_coeffs, SH_NUM_BANDS, reinterpret_cast<const GLfloat*>(coeffs));
	}

	inline void set_camera_position(const glm::vec3& camera_position) const {
		glUniform3f(uni_camera_position, camera_position.x, camera_position.y, camera_position.z);
	}

private:
	const GLuint uni_brdf_coeffs;
	const GLuint uni_camera_position;
};
