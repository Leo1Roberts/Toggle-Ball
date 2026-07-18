#ifndef SHADER_H
#define SHADER_H

#include "main.h"
#include "GLUtilities.h"
#include "VectorMatrix.h"

#include <memory>

class Shader {
public:
	Shader(const std::string& vertexSource, const std::string& fragmentSource);

	Shader(const Shader&) = delete;
	Shader& operator=(const Shader&) = delete;

	void use() const { glUseProgram(program); }

	void setFloat(const std::string& name, float value) const {
		glUniform1f(glGetUniformLocation(program, name.c_str()), value);
	}
	void setVec2(const std::string& name, vec2 value) const {
		glUniform2fv(glGetUniformLocation(program, name.c_str()), 1, &value.x);
	}
	void setVec3(const std::string& name, vec3 value) const {
		glUniform3fv(glGetUniformLocation(program, name.c_str()), 1, &value.x);
	}
	void setVec4(const std::string& name, vec4 value) const {
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

	[[nodiscard]] static GLShader compileShader(GLenum type, const std::string& source);
};

namespace Shaders {
	extern std::unique_ptr<Shader> button;
	extern std::unique_ptr<Shader> cursor;
	extern std::unique_ptr<Shader> object;
	extern std::unique_ptr<Shader> outline;
	extern std::unique_ptr<Shader> quad;
	extern std::unique_ptr<Shader> text;

	void load();
}

#endif // SHADER_H