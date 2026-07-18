#include "Shader.h"
#include "AssetManager.h"

#if defined(PLATFORM_ANDROID)
#include <android/log.h>
#endif


Shader::Shader(const std::string& vertexSource, const std::string& fragmentSource) {
	std::string version;
#if defined(PLATFORM_DESKTOP)
	version = "#version 420 core\n";
#else
	version = "#version 300 es\n";
#endif
	GLShader vs = compileShader(GL_VERTEX_SHADER, version + vertexSource);
	GLShader fs = compileShader(GL_FRAGMENT_SHADER, version + fragmentSource);

	program = GLProgram(glCreateProgram());

	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);

	GLint success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		GLint logLength = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
		if (logLength > 0) {
			std::vector<char> log(logLength);
			glGetProgramInfoLog(program, logLength, nullptr, log.data());
#if defined(PLATFORM_ANDROID)
			__android_log_print(ANDROID_LOG_ERROR, "Shader", "Link Error: %s", log.data());
#endif
		} else {
#if defined(PLATFORM_ANDROID)
			__android_log_print(ANDROID_LOG_ERROR, "Shader", "Link failed with no log.");
#endif
		}
		char infoLog[512];
		glGetProgramInfoLog(program, 512, NULL, infoLog);
	}
}

GLShader Shader::compileShader(GLenum type, const std::string& source) {
	GLuint id = glCreateShader(type);
	const char* src = source.c_str();
	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);

	return GLShader(id);
}

namespace Shaders {
	std::unique_ptr<Shader> button;
	std::unique_ptr<Shader> cursor;
	std::unique_ptr<Shader> object;
	std::unique_ptr<Shader> outline;
	std::unique_ptr<Shader> quad;
	std::unique_ptr<Shader> text;

	void load() {
		button = std::make_unique<Shader>(AssetManager::loadTextFile("shaders/button.vert"), AssetManager::loadTextFile("shaders/button.frag"));
		cursor = std::make_unique<Shader>(AssetManager::loadTextFile("shaders/cursor.vert"), AssetManager::loadTextFile("shaders/cursor.frag"));
		object = std::make_unique<Shader>(AssetManager::loadTextFile("shaders/object.vert"), AssetManager::loadTextFile("shaders/object.frag"));
		outline = std::make_unique<Shader>(AssetManager::loadTextFile("shaders/outline.vert"), AssetManager::loadTextFile("shaders/outline.frag"));
		quad = std::make_unique<Shader>(AssetManager::loadTextFile("shaders/quad.vert"), AssetManager::loadTextFile("shaders/quad.frag"));
		text = std::make_unique<Shader>(AssetManager::loadTextFile("shaders/text.vert"), AssetManager::loadTextFile("shaders/text.frag"));
	}
}