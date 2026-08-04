#ifndef SHADER_H
#define SHADER_H

#include "GLUtilities.h"

#include "glm/gtc/type_ptr.hpp"
#include <glm/glm.hpp>
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
	void setVec2(const std::string& name, glm::vec2 vec) const {
		glUniform2fv(glGetUniformLocation(program, name.c_str()), 1, glm::value_ptr(vec));
	}
	void setVec3(const std::string& name, glm::vec3 vec) const {
		glUniform3fv(glGetUniformLocation(program, name.c_str()), 1, glm::value_ptr(vec));
	}
	void setVec4(const std::string& name, glm::vec4 vec) const {
		glUniform4fv(glGetUniformLocation(program, name.c_str()), 1, glm::value_ptr(vec));
	}
	void setMat3(const std::string& name, const glm::mat3& mat, bool transpose = false) const {
		glUniformMatrix3fv(glGetUniformLocation(program, name.c_str()), 1, transpose, glm::value_ptr(mat));
	}
	void setMat4(const std::string& name, const glm::mat4& mat, bool transpose = false) const {
		glUniformMatrix4fv(glGetUniformLocation(program, name.c_str()), 1, transpose, glm::value_ptr(mat));
	}

private:
	GLProgram program;

	[[nodiscard]] static GLShader compileShader(GLenum type, const std::string& source);
};

namespace Shaders {
	extern std::unique_ptr<Shader> cursor;
	extern std::unique_ptr<Shader> object;
	extern std::unique_ptr<Shader> outline;
	extern std::unique_ptr<Shader> panel;
	extern std::unique_ptr<Shader> quad;
	extern std::unique_ptr<Shader> text;

	void load();
}

#endif // SHADER_H