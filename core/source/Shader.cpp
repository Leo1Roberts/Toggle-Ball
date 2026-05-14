#include "Shader.h"

GLShader Shader::compileShader(GLenum type, const std::string& source) const {
	GLuint id = glCreateShader(type);
	const char* src = source.c_str();
	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);
	CHECK_ERROR();

	return GLShader(id);
}