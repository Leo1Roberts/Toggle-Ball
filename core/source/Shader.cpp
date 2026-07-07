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
	std::unique_ptr<Shader> object;
	std::unique_ptr<Shader> outline;
	std::unique_ptr<Shader> button;
	std::unique_ptr<Shader> text;
	std::unique_ptr<Shader> cursor;

	void load() {
		object = std::make_unique<Shader>(AssetManager::loadTextFile("shaders/object.vert"), AssetManager::loadTextFile("shaders/object.frag"));
		outline = std::make_unique<Shader>(AssetManager::loadTextFile("shaders/objectOutline.vert"), AssetManager::loadTextFile("shaders/objectOutline.frag"));
		button = std::make_unique<Shader>(AssetManager::loadTextFile("shaders/button.vert"), AssetManager::loadTextFile("shaders/button.frag"));
		text = std::make_unique<Shader>(AssetManager::loadTextFile("shaders/text.vert"), AssetManager::loadTextFile("shaders/text.frag"));
		cursor = std::make_unique<Shader>(AssetManager::loadTextFile("shaders/cursor.vert"), AssetManager::loadTextFile("shaders/cursor.frag"));
	}
}