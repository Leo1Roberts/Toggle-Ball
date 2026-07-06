#include "Texture.h"
#include "AssetManager.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Texture::Texture(const std::string& path, bool monochrome) {
	std::vector<byte> buffer = AssetManager::loadAssetToBuffer(path);

	glGenTextures(1, texture.ptr());
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, channels;
	int desiredChannels = monochrome ? STBI_grey : STBI_rgb_alpha;

	unsigned char* data = stbi_load_from_memory(buffer.data(), static_cast<int>(buffer.size()),
	                                            &width, &height, &channels, desiredChannels);
	if (!data) {
		unbind();
		throw std::runtime_error("Failed to decode texture: " + path);
	}

	GLint internalFormat = monochrome ? GL_R8 : GL_SRGB8_ALPHA8;
	GLenum format = monochrome ? GL_RED : GL_RGBA;

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);
	unbind();
}

namespace Textures {
	std::unique_ptr<Texture> white;
	std::unique_ptr<Texture> basketball;

	namespace Cursors {
		std::unique_ptr<Texture> arrow;
		std::unique_ptr<Texture> hand;
		std::unique_ptr<Texture> resize;
	} // namespace Cursors

	void load() {
		white = std::make_unique<Texture>("textures/white.png");
		basketball = std::make_unique<Texture>("textures/Ball.png");
#if defined(PLATFORM_DESKTOP)
		Cursors::arrow = std::make_unique<Texture>("cursors/arrow.png");
		Cursors::hand = std::make_unique<Texture>("cursors/hand.png");
		Cursors::resize = std::make_unique<Texture>("cursors/resize.png");
#endif
	}
} // namespace Textures