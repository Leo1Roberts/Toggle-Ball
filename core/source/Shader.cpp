#include "Shader.h"

GLShader Shader::compileShader(GLenum type, const std::string& source) const {
	GLuint id = glCreateShader(type);
	const char* src = source.c_str();
	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);

	return GLShader(id);
}

namespace Shaders {
	std::unique_ptr<Shader> object;
	std::unique_ptr<Shader> outline;
// TODO: prefix shader code with OpenGL version
#ifdef WINDOWS_VERSION
	void load() {
		object = std::make_unique<Shader>(importTextFile(ASSETS_PATH + "shaders/object.vert"), importTextFile(ASSETS_PATH + "shaders/object.frag"));
		outline = std::make_unique<Shader>(importTextFile(ASSETS_PATH + "shaders/objectOutline.vert"), importTextFile(ASSETS_PATH + "shaders/objectOutline.frag"));
	}
#else
	void load(AAssetManager* assetManager) {
		object = std::make_unique<Shader>(importTextFile(assetManager, "shaders/object.vert"), importTextFile(assetManager, "shaders/object.frag"));
		outline = std::make_unique<Shader>(importTextFile(assetManager, "shaders/objectOutline.vert"), importTextFile(assetManager, "shaders/objectOutline.frag"));
	}
#endif
}