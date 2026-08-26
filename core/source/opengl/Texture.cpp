#include "opengl/Texture.h"
#include "utilities/AssetManager.h"
#include "ui/Font.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


Texture::Texture(const std::string& path, bool monochrome, bool saveData) {
	std::vector<byte> buffer = AssetManager::loadAssetToBuffer(path);

	glGenTextures(1, texture.ptr());
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int channels;
	int desiredChannels = monochrome ? STBI_grey : STBI_rgb_alpha;

	unsigned char* rawData = stbi_load_from_memory(buffer.data(), (int)buffer.size(),
	                                            &width, &height, &channels, desiredChannels);
	if (!rawData) {
		unbind();
		buffer.clear();
		throw std::runtime_error("Failed to decode texture: " + path);
	}

	GLint internalFormat = monochrome ? GL_R8 : GL_SRGB8_ALPHA8;
	GLenum format = monochrome ? GL_RED : GL_RGBA;

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, rawData);
	glGenerateMipmap(GL_TEXTURE_2D);

	if (saveData)
		data.assign(rawData, rawData + width * height * desiredChannels);

	stbi_image_free(rawData);
	unbind();
}

namespace Textures {
	std::unique_ptr<Texture> white;
	std::unique_ptr<Texture> basketball;

	void load() {
		white = std::make_unique<Texture>("textures/white.png");
		basketball = std::make_unique<Texture>("textures/basketball.png");
	}
}