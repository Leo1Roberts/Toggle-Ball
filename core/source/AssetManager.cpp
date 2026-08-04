#include "AssetManager.h"

#include <filesystem>
#include <fstream>


namespace AssetManager {
#if defined(PLATFORM_ANDROID)
	static AAssetManager* androidAssetManager = nullptr;
	void init(AAssetManager* mgr) {
		androidAssetManager = mgr;
	}
#elif defined(PLATFORM_DESKTOP)
	constexpr std::string_view ASSETS_PATH = ASSETS_PATH_MACRO;
#endif

	std::vector<byte> loadAssetToBuffer(const std::string& path, FileType type) {
		std::vector<byte> buffer;

#if defined(PLATFORM_ANDROID)
		if (!androidAssetManager) return buffer;

		AAsset* asset = AAssetManager_open(androidAssetManager, path.c_str(), AASSET_MODE_BUFFER);
		if (asset) {
			size_t size = AAsset_getLength(asset);
			buffer.resize(size);
			AAsset_read(asset, buffer.data(), size);
			AAsset_close(asset);
		}

#elif defined(PLATFORM_DESKTOP)
		std::string fullPath = std::string(ASSETS_PATH) + path;

		if (std::ifstream file(fullPath, type == FileType::Binary ? std::ios::binary | std::ios::ate : std::ios::ate); file.is_open()) {
			const std::streamsize size = file.tellg();
			file.seekg(0, std::ios::beg);

			buffer.resize(size);
			file.read(reinterpret_cast<char*>(buffer.data()), size);

			buffer.resize(file.gcount());
		}
#endif

		if (buffer.empty())
			throw std::runtime_error("Could not load asset: " + path);

		return buffer;
	}

	std::string loadTextFile(const std::string& path) {
		std::vector<byte> buffer = loadAssetToBuffer(path, FileType::Text);
		return {buffer.begin(), buffer.end()};
	}


	bool saveTextFile(const std::string& path, const std::string& text) {
#if defined(PLATFORM_DESKTOP)
		std::ofstream ofs(std::string(ASSETS_PATH) + path);
		if (ofs.is_open()) {
			ofs << text;
			ofs.close();
			return true;
		}
#endif
		return false;
	}


	std::vector<std::string> getFileList(const std::string& directory, const std::string& extension) {
		std::vector<std::string> fileList;

#if defined(PLATFORM_ANDROID)
		AAssetDir* dir = AAssetManager_openDir(androidAssetManager, directory.c_str());
		if (dir) {
			while (const char* fName = AAssetDir_getNextFileName(dir)) {
				std::string fileName = fName;

				if (fileName.length() > extension.length() &&
				    fileName.compare(fileName.length() - extension.length(), extension.length(), extension) == 0) {

					// Strip the extension out before returning
					fileList.push_back(fileName.substr(0, fileName.length() - extension.length()));
				}
			}
			AAssetDir_close(dir);
		}
#else
		std::filesystem::path searchPath = std::filesystem::path(ASSETS_PATH) / directory;

		if (std::filesystem::exists(searchPath) && std::filesystem::is_directory(searchPath))
			for (const auto& entry: std::filesystem::directory_iterator(searchPath))
				if (entry.is_regular_file())
					if (const std::filesystem::path& p = entry.path(); p.extension() == extension)
						fileList.push_back(p.stem().string());
#endif

		return fileList;
	}
}