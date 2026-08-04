#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include "TypeAliases.h"

#if defined(PLATFORM_ANDROID)
#include <android/asset_manager.h>
#endif
#include <string>
#include <vector>

namespace AssetManager {
	enum class FileType { Binary, Text };

#if defined(PLATFORM_ANDROID)
	void init(AAssetManager* mgr);
#endif

	std::vector<byte> loadAssetToBuffer(const std::string& path, FileType type = FileType::Binary);
	std::string loadTextFile(const std::string& path);

	bool saveTextFile(const std::string& path, const std::string& text);

	std::vector<std::string> getFileList(const std::string& directory, const std::string& extension);
}

#endif