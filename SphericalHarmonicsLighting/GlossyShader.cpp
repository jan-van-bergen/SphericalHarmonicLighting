#include "MeshShaders.h"

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
		SH::calc_phong_lobe_coeffs(phong_lobe_coeffs);

		bind();
		{
			glUniform1fv(uni_phong_lobe_coeffs, SH_NUM_BANDS, phong_lobe_coeffs);
		}
		unbind();
	};
