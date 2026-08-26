#ifndef TEXTURE_H
#define TEXTURE_H

#include "opengl/GLUtilities.h"

#include <memory>
#include <vector>


class Texture {
public:
	explicit Texture(const std::string& fileName, bool monochrome = false, bool saveData = false);

	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;

	void bind(unsigned int i) const {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, texture);
	}
	static void unbind() {
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	[[nodiscard]] unsigned char* getData() { return data.data(); }
	[[nodiscard]] int getWidth() const { return width; }
	[[nodiscard]] int getHeight() const { return height; }

private:
	GLTexture texture;
	std::vector<unsigned char> data;
	int width, height;
};

namespace Textures {
	extern std::unique_ptr<Texture> white;
	extern std::unique_ptr<Texture> basketball;

	void load();
}

#endif // TEXTURE_H
