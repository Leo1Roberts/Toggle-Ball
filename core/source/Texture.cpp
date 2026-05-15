#include "Texture.h"

#ifdef WINDOWS_VERSION
#include <stb/stb_image.h>

Texture::Texture(const std::string& path, bool monochrome) {
	glGenTextures(1, texture.ptr());
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, channels;

	unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, monochrome ? STBI_grey : STBI_rgb_alpha);

	if (data) {
		GLint internalFormat = monochrome ? GL_R8 : GL_SRGB8_ALPHA8;
		GLenum format = monochrome ? GL_RED : GL_RGBA;
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);

		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(data);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
}

#else

Texture::Texture(AAssetManager* assetManager, const std::string& path, bool monochrome) {
	glGenTextures(1, texture.ptr());
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	AAsset* image = AAssetManager_open(assetManager, path.c_str(), AASSET_MODE_BUFFER);
	AImageDecoder* decoder = nullptr;
	AImageDecoder_createFromAAsset(image, &decoder);
	int32_t bitmapFormat = (monochrome) ? ANDROID_BITMAP_FORMAT_A_8 : ANDROID_BITMAP_FORMAT_RGBA_8888;
	AImageDecoder_setAndroidBitmapFormat(decoder, bitmapFormat);

	const AImageDecoderHeaderInfo* header = nullptr;
	header = AImageDecoder_getHeaderInfo(decoder);
	int width = AImageDecoderHeaderInfo_getWidth(header);
	int height = AImageDecoderHeaderInfo_getHeight(header);
	size_t stride = AImageDecoder_getMinimumStride(decoder);

	auto data = std::vector<uint8_t>(height * stride);
	AImageDecoder_decodeImage(
	decoder,
	data.data(),
	stride,
	data.size());

	GLint internalFormat = monochrome ? GL_R8 : GL_SRGB8_ALPHA8;
	GLenum format = monochrome ? GL_RED : GL_RGBA;
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data.data());

	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);
}

#endif
