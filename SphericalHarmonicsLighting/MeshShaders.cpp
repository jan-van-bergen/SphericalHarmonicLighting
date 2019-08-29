#include "MeshShaders.h"

const int diffuse_define_count = 1;
const char * diffuse_define_names[diffuse_define_count] = {
	"SH_COEFFICIENT_COUNT"
};
const char * diffuse_define_definitions[diffuse_define_count] = {
	TO_STRING(SH_COEFFICIENT_COUNT)
};
const Shader::Defines diffuse_defines(diffuse_define_count, diffuse_define_names, diffuse_define_definitions);

DiffuseShader::DiffuseShader() : 
	MeshShader(Type::DIFFUSE, DATA_PATH("Shaders/vertex_diffuse.glsl"), DATA_PATH("Shaders/fragment.glsl"), nullptr, diffuse_defines) 
	{ };

const int glossy_define_count = 2;
const char * glossy_define_names[glossy_define_count] = {
	"SH_NUM_BANDS",
	"SH_COEFFICIENT_COUNT"
};
const char * glossy_define_definitions[glossy_define_count] = {
	TO_STRING(SH_NUM_BANDS),
	TO_STRING(SH_COEFFICIENT_COUNT)
};
const Shader::Defines glossy_defines(glossy_define_count, glossy_define_names, glossy_define_definitions);

GlossyShader::GlossyShader() : 
	MeshShader(Type::GLOSSY, DATA_PATH("Shaders/vertex_glossy.glsl"), DATA_PATH("Shaders/fragment.glsl"), nullptr, glossy_defines),

	uni_phong_lobe_coeffs(get_uniform("phong_lobe_coeffs")),
	uni_camera_position  (get_uniform("camera_position"))
	{ 
		float phong_lobe_coeffs[SH_NUM_BANDS];
		calc_phong_lobe_coeffs(phong_lobe_coeffs);

		bind();
		{
			glUniform1fv(uni_phong_lobe_coeffs, SH_NUM_BANDS, phong_lobe_coeffs);
		}
		unbind();
	};

