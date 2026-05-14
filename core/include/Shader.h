#ifndef SHADER_H
#define SHADER_H

#include "main.h"
#include "GLUtilities.h"

class Shader {
public:
	Shader(const std::string& vertexSource, const std::string& fragmentSource) {
		GLShader vs = compileShader(GL_VERTEX_SHADER, vertexSource);
		GLShader fs = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

		program = GLProgram(glCreateProgram());

		glAttachShader(program, vs);
		glAttachShader(program, fs);
		glLinkProgram(program);

		CHECK_ERROR();
	}

	void use() const { glUseProgram(program); }

	void setFloat(const std::string& name, float value) const {
		glUniform1f(glGetUniformLocation(program, name.c_str()), value);
	}
	void setVec3(const std::string& name, const vec3& value) const {
		glUniform3fv(glGetUniformLocation(program, name.c_str()), 1, &value.x);
	}
	void setVec4(const std::string& name, const vec4& value) const {
		glUniform4fv(glGetUniformLocation(program, name.c_str()), 1, &value.r);
	}
	void setMat3(const std::string& name, const mat3& mat, bool transpose = false) const {
		glUniformMatrix3fv(glGetUniformLocation(program, name.c_str()), 1, transpose, &mat.a);
	}
	void setMat4(const std::string& name, const mat4& mat, bool transpose = false) const {
		glUniformMatrix4fv(glGetUniformLocation(program, name.c_str()), 1, transpose, mat.m16);
	}

private:
	GLProgram program;

	GLShader compileShader(GLenum type, const std::string& source) const;
};

#endif // SHADER_H