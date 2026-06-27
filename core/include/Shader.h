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
	}

	void use() const { glUseProgram(program); }

	void setFloat(const std::string& name, float value) const {
		use();
		glUniform1f(glGetUniformLocation(program, name.c_str()), value);
	}
	void setVec2(const std::string& name, const vec2& value) const {
		use();
		glUniform2fv(glGetUniformLocation(program, name.c_str()), 1, &value.x);
	}
	void setVec3(const std::string& name, const vec3& value) const {
		use();
		glUniform3fv(glGetUniformLocation(program, name.c_str()), 1, &value.x);
	}
	void setVec4(const std::string& name, const vec4& value) const {
		use();
		glUniform4fv(glGetUniformLocation(program, name.c_str()), 1, &value.r);
	}
	void setMat3(const std::string& name, const mat3& mat, bool transpose = false) const {
		use();
		glUniformMatrix3fv(glGetUniformLocation(program, name.c_str()), 1, transpose, &mat.a);
	}
	void setMat4(const std::string& name, const mat4& mat, bool transpose = false) const {
		use();
		glUniformMatrix4fv(glGetUniformLocation(program, name.c_str()), 1, transpose, mat.m16);
	}

private:
	GLProgram program;

	[[nodiscard]] static GLShader compileShader(GLenum type, const std::string& source) ;
};

namespace Shaders {
	extern std::unique_ptr<Shader> object;
	extern std::unique_ptr<Shader> outline;
	extern std::unique_ptr<Shader> button;
	extern std::unique_ptr<Shader> text;
	extern std::unique_ptr<Shader> cursor;

	void load();
}

#endif // SHADER_H