#ifndef TEXTURE_H
#define TEXTURE_H

#include "GLUtilities.h"

class Texture {
public:
	Texture(const std::string& fileName, bool monochrome = false);

	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;

	void bind(unsigned int i) const {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, texture);
	}

	static void unbind() {
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

	void load();
}

#endif // TEXTURE_H
