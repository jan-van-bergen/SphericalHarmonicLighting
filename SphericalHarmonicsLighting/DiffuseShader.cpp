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
