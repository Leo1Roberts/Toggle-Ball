#ifndef TEXTURE_H
#define TEXTURE_H

#include "GLUtilities.h"

class Texture {
public:
#ifdef WINDOWS_VERSION
	Texture(const std::string& path, bool monochrome);
#else
	Texture(AAssetManager* assetManager, const std::string& path, bool monochrome);
#endif

	void bind(unsigned int i) const {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, texture);
	}

	void unbind() const {
		glBindTexture(GL_TEXTURE_2D, 0);
	}

private:
	GLTexture texture;
};

namespace Textures {
	extern std::unique_ptr<Texture> white;
	extern std::unique_ptr<Texture> basketball;

	namespace Cursors {
		extern std::unique_ptr<Texture> arrow;
		extern std::unique_ptr<Texture> hand;
		extern std::unique_ptr<Texture> resize;
	}

#ifdef WINDOWS_VERSION
	void init();
#else
	void init(AAssetManager* assetManager);
#endif
}

#endif // TEXTURE_H
