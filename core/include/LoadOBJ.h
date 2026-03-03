#ifndef LOAD_OBJ_H
#define LOAD_OBJ_H

#include "Model.h"

struct FileIndices {
	int v;
	int vt;
	int vn;

	bool equals(const FileIndices& other) {
		return v == other.v && vt == other.vt && vn == other.vn;
	}
};

int addFileIndices(std::vector<FileIndices>* fileIndices, const FileIndices* newIndices);

#ifdef WINDOWS_VERSION
bool loadOBJ(const std::string& path, std::vector<Vertex>* vertices, std::vector<Index>* indices, const col& color = WHITE);
#else

bool loadOBJ(AAssetManager* assetManager, const std::string& path, std::vector<Vertex>* vertices,
             std::vector<Index>* indices, const col& color = WHITE);

#endif

#endif// LOAD_OBJ_H
