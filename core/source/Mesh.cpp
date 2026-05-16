#include "main.h"
#include "Mesh.h"

void ObjectVertex::setupLayout() {
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ObjectVertex), (void*)offsetof(ObjectVertex, position));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ObjectVertex), (void*)offsetof(ObjectVertex, uv));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(ObjectVertex), (void*)offsetof(ObjectVertex, normal));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ObjectVertex), (void*)offsetof(ObjectVertex, color));
	glEnableVertexAttribArray(3);
}

struct FileIndices {
	int v;
	int vt;
	int vn;

	bool equals(const FileIndices& other) {
		return v == other.v && vt == other.vt && vn == other.vn;
	}
};

int addFileIndices(std::vector<FileIndices>* fileIndices, const FileIndices* newIndices) {
	for (int i = 0; i < fileIndices->size(); i++) {
		if (fileIndices->at(i).equals(*newIndices)) return i;
	}

	fileIndices->push_back(*newIndices);
	return (int)fileIndices->size() - 1;
}

template <>
#ifdef WINDOWS_VERSION
Mesh<ObjectVertex>::Mesh(const std::string& path, col color) : Mesh(GL_STATIC_DRAW) {
	std::vector<vec3> tempVertices;
	std::vector<vec2> tempUVs;
	std::vector<vec3> tempNormals;
	std::vector<FileIndices> fileIndices;

	std::vector<ObjectVertex> vertices;
	std::vector<Index> indices;

	FILE* file;
	fopen_s(&file, path.c_str(), "r");
	if (!file) throw std::runtime_error("Failed to open file: " + path);

	while (true) {
		char lineHeader[128] = {0};
		int res = fscanf_s(file, "%s", lineHeader, (unsigned int)(sizeof(char) * 128));

		if (res == EOF) break;
		if (strcmp(lineHeader, "v") == 0) {
			vec3 vertex;
			fscanf_s(file, "%f %f %f", &vertex.x, &vertex.y, &vertex.z);
			tempVertices.push_back(vertex);
		} else if (strcmp(lineHeader, "vt") == 0) {
			vec2 uv;
			fscanf_s(file, "%f %f", &uv.x, &uv.y);
			tempUVs.push_back(uv);
		} else if (strcmp(lineHeader, "vn") == 0) {
			vec3 normal;
			fscanf_s(file, "%f %f %f", &normal.x, &normal.y, &normal.z);
			tempNormals.push_back(normal);
		} else if (strcmp(lineHeader, "f") == 0) {
			FileIndices triangleIndices[3];
			for (int i = 0; i < 3; i++) {
				fscanf_s(file, "%d/%d/%d", &triangleIndices[i].v, &triangleIndices[i].vt, &triangleIndices[i].vn);
				triangleIndices[i].v--;
				triangleIndices[i].vt--;
				triangleIndices[i].vn--;
				indices.push_back(addFileIndices(&fileIndices, &triangleIndices[i]));
			}
		}
	}

	for (int i = 0; i < fileIndices.size(); i++) {
		vertices.emplace_back(tempVertices[fileIndices[i].v], tempUVs[fileIndices[i].vt], tempNormals[fileIndices[i].vn], color);
	}

	fclose(file);
	setData(vertices, indices);
}
#else
Mesh<ObjectVertex>::Mesh(AAssetManager* assetManager, const std::string& path, col color) : Mesh(GL_STATIC_DRAW) {
	std::vector<vec3> tempVertices;
	std::vector<vec2> tempUVs;
	std::vector<vec3> tempNormals;
	std::vector<FileIndices> fileIndices;

	std::vector<ObjectVertex> vertices;
	std::vector<Index> indices;

	AAsset* file = AAssetManager_open(assetManager, path.c_str(), AASSET_MODE_BUFFER);
	if (!file) throw std::runtime_error("Failed to open asset: " + path);

	const off_t length = AAsset_getLength(file);

	const char* buffer = (const char*)AAsset_getBuffer(file);
	int count = 0;

	while (true) {
		int offset = 0;
		char line[1000] = {0};
		char c = 0;
		do {
			if (count == length) {
				offset = EOF;
				break;
			}
			c = *(buffer + count);
			line[offset++] = c;
			count++;
		} while (c != '\n');

		char header[3] = {0};
		header[0] = line[0];
		header[1] = line[1];

		if (strcmp(header, "v ") == 0) {
			vec3 vertex;
			sscanf(line, "v %f %f %f", &vertex.x, &vertex.y, &vertex.z);
			tempVertices.push_back(vertex);
		} else if (strcmp(header, "vt") == 0) {
			vec2 uv;
			sscanf(line, "vt %f %f", &uv.x, &uv.y);
			tempUVs.push_back(uv);
		} else if (strcmp(header, "vn") == 0) {
			vec3 normal;
			sscanf(line, "vn %f %f %f", &normal.x, &normal.y, &normal.z);
			tempNormals.push_back(normal);
		} else if (strcmp(header, "f ") == 0) {
			FileIndices triangleIndices[3];
			sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
			       &triangleIndices[0].v, &triangleIndices[0].vt, &triangleIndices[0].vn,
			       &triangleIndices[1].v, &triangleIndices[1].vt, &triangleIndices[1].vn,
			       &triangleIndices[2].v, &triangleIndices[2].vt, &triangleIndices[2].vn);

			for (FileIndices& triangleIndex: triangleIndices) {
				triangleIndex.v--;
				triangleIndex.vt--;
				triangleIndex.vn--;
				indices.push_back(addFileIndices(&fileIndices, &triangleIndex));
			}
		}

		if (offset == EOF) break;
	}

	size_t size = fileIndices.size();
	for (int i = 0; i < size; i++)
		vertices.emplace_back(tempVertices[fileIndices[i].v], tempUVs[fileIndices[i].vt], tempNormals[fileIndices[i].vn], color);

	AAsset_close(file);
	setData(vertices, indices);
}
#endif

namespace Meshes {
	std::unique_ptr<Mesh<ObjectVertex>> ball;
	std::unique_ptr<Mesh<ObjectVertex>> plane;

#ifdef WINDOWS_VERSION
	void load() {
		ball = std::make_unique<Mesh<ObjectVertex>>(ASSETS_PATH + "models/Ball.obj");
		plane = std::make_unique<Mesh<ObjectVertex>>(ASSETS_PATH + "models/Ground.obj", BOUNDARY);
	}
#else
	void load(AAssetManager* assetManager) {
		ball = std::make_unique<Mesh<ObjectVertex>>(assetManager, "models/Ball.obj");
		plane = std::make_unique<Mesh<ObjectVertex>>(assetManager, "models/Ground.obj", BOUNDARY);
	}
#endif
}