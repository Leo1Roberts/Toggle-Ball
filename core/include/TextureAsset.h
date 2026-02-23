#ifndef TEXTURE_ASSET_H
#define TEXTURE_ASSET_H

struct TextureAsset {
	GLuint textureID;
	int width, height;

#ifdef WINDOWS_VERSION
	/*!
	 * Loads a texture asset from the assets/ directory
	 * @param assetPath The path to the asset
	 * @return a shared pointer to a texture asset, resources will be reclaimed when it's cleaned up
	 */
	static TextureAsset *loadAsset(const std::string& assetPath, bool monochrome);
#else

	/*!
	 * Loads a texture asset from the assets/ directory
	 * @param assetManager Asset manager to use
	 * @param assetPath The path to the asset
	 * @return a shared pointer to a texture asset, resources will be reclaimed when it's cleaned up
	 */
	static TextureAsset* loadAsset(AAssetManager* assetManager, const std::string& assetPath, bool monochrome);

#endif

	~TextureAsset();

private:
	inline TextureAsset(GLuint iTextureId, int iWidth, int iHeight) : textureID(iTextureId),
	                                                                  width(iWidth),
	                                                                  height(iHeight) {}
};

extern TextureAsset* whiteTex;
extern TextureAsset* basketballTex;

#endif // TEXTURE_ASSET_H