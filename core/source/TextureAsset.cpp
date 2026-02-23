#include "main.h"
#include "ImageUtils.h"
#include "MatrixUtilities.h"
#include "TextureAsset.h"

#ifdef WINDOWS_VERSION
TextureAsset *TextureAsset::loadAsset(const std::string& assetPath, bool monochrome) {
	// Load the image
	int width, height, channels;
	int bitmapFormat = monochrome ? grey : RGBA;
	auto upImageData = std::unique_ptr<unsigned char>(
		loadImage(assetPath.c_str(), &width, &height, &channels, bitmapFormat));

	GLuint textureId;
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Load the texture into VRAM
	GLint internalFormat = monochrome ? GL_R8 : GL_SRGB8_ALPHA8;
	GLenum format = monochrome ? GL_RED : GL_RGBA;
	glTexImage2D(
		GL_TEXTURE_2D, // target
		0, // mip level
		internalFormat, // internal format, often advisable to use BGR
		width, // width of the texture
		height, // height of the texture
		0, // border (always 0)
		format, // format
		GL_UNSIGNED_BYTE, // type
		upImageData.get() // Data to upload
	);
	// generate mip levels. Not really needed for 2D, but good to do
	glGenerateMipmap(GL_TEXTURE_2D);

	return new TextureAsset(textureId, width, height);
}
#else

TextureAsset* TextureAsset::loadAsset(AAssetManager* assetManager, const std::string& assetPath, bool monochrome) {
	// Get the image from asset manager
	auto image = AAssetManager_open(
			assetManager,
			assetPath.c_str(),
			AASSET_MODE_BUFFER);

	// Make a decoder to turn it into a texture
	AImageDecoder* pAndroidDecoder = nullptr;
	AImageDecoder_createFromAAsset(image, &pAndroidDecoder);

	// make sure we get 8 bits per channel out. RGBA order.
	int32_t bitmapFormat = (monochrome) ? ANDROID_BITMAP_FORMAT_A_8 : ANDROID_BITMAP_FORMAT_RGBA_8888;
	AImageDecoder_setAndroidBitmapFormat(pAndroidDecoder, bitmapFormat);

	// Get the image header, to help set everything up
	const AImageDecoderHeaderInfo* pAndroidHeader = nullptr;
	pAndroidHeader = AImageDecoder_getHeaderInfo(pAndroidDecoder);

	// important metrics for sending to GL
	auto width = AImageDecoderHeaderInfo_getWidth(pAndroidHeader);
	auto height = AImageDecoderHeaderInfo_getHeight(pAndroidHeader);
	auto stride = AImageDecoder_getMinimumStride(pAndroidDecoder);

	// Get the bitmap data of the image
	auto upAndroidImageData = std::make_unique<std::vector<uint8_t>>(height * stride);
	AImageDecoder_decodeImage(
			pAndroidDecoder,
			upAndroidImageData->data(),
			stride,
			upAndroidImageData->size());

	// Get an opengl texture
	GLuint textureId;
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Load the texture into VRAM
	GLint internalFormat = (monochrome) ? GL_R8 : GL_SRGB8_ALPHA8;
	GLenum format = (monochrome) ? GL_RED : GL_RGBA;
	glTexImage2D(
			GL_TEXTURE_2D, // target
			0, // mip level
			internalFormat, // internal bitmapFormat, often advisable to use BGR
			width, // width of the texture
			height, // height of the texture
			0, // border (always 0)
			format, // bitmapFormat
			GL_UNSIGNED_BYTE, // type
			upAndroidImageData->data() // Data to upload
	);

	glGenerateMipmap(GL_TEXTURE_2D);

	// cleanup helpers
	AImageDecoder_delete(pAndroidDecoder);
	AAsset_close(image);

	return new TextureAsset(textureId, width, height);
}

#endif

TextureAsset* whiteTex;
TextureAsset* basketballTex;

TextureAsset::~TextureAsset() {
	// return texture resources
	glDeleteTextures(1, &textureID);
	textureID = 0;
}