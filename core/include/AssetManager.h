#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include "main.h"

#include <vector>

namespace AssetManager {
#if defined(PLATFORM_ANDROID)
	void init(AAssetManager* mgr);
#endif

	std::vector<byte> loadAssetToBuffer(const std::string& path);
	std::string loadTextFile(const std::string& path);

	bool saveTextFile(const std::string& path, const std::string& text);

	std::vector<std::string> getFileList(const std::string& directory, const std::string& extension);
} // namespace AssetManager

#endif