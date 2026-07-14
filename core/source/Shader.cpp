#include "Shader.h"
#include "AssetManager.h"

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