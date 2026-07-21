#include "main.h"
#include "Mesh.h"

#include "AssetManager.h"

#include <glm/glm.hpp>
#include <sstream>

void ObjectVertex::setupLayout() {
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ObjectVertex), reinterpret_cast<void*>(offsetof(ObjectVertex, position)));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ObjectVertex), reinterpret_cast<void*>(offsetof(ObjectVertex, uv)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(ObjectVertex), reinterpret_cast<void*>(offsetof(ObjectVertex, normal)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ObjectVertex), reinterpret_cast<void*>(offsetof(ObjectVertex, color)));
	glEnableVertexAttribArray(3);
}

struct FileIndices {
	int v;
	int vt;
	int vn;

	[[nodiscard]] bool equals(const FileIndices& other) const {
		return v == other.v && vt == other.vt && vn == other.vn;
	}
};

int addFileIndices(std::vector<FileIndices>* fileIndices, const FileIndices* newIndices) {
	for (int i = 0; i < fileIndices->size(); i++) {
		if (fileIndices->at(i).equals(*newIndices)) return i;
	}

	fileIndices->push_back(*newIndices);
	return static_cast<int>(fileIndices->size()) - 1;
}

template<>
Mesh<ObjectVertex>::Mesh(const std::string& path, col color) : Mesh(GL_STATIC_DRAW) {
	std::vector<glm::vec3> tempVertices;
	std::vector<glm::vec2> tempUVs;
	std::vector<glm::vec3> tempNormals;
	std::vector<FileIndices> fileIndices;

	std::vector<ObjectVertex> vertices;
	std::vector<Index> indices;

	std::vector<byte> data = AssetManager::loadAssetToBuffer(path);
	if (data.empty())
		throw std::runtime_error("Failed to load mesh asset: " + path);

	std::string fileContent(data.begin(), data.end());
	std::istringstream stream(fileContent);
	std::string line;

	while (std::getline(stream, line)) {
		if (line.empty() || line[0] == '#') continue;

		std::istringstream lineStream(line);
		std::string prefix;

		if (!(lineStream >> prefix)) continue;

		if (prefix == "v") {
			glm::vec3 vertex;
			lineStream >> vertex.x >> vertex.y >> vertex.z;
			tempVertices.push_back(vertex);
		} else if (prefix == "vt") {
			glm::vec2 uv;
			lineStream >> uv.x >> uv.y;
			tempUVs.push_back(uv);
		} else if (prefix == "vn") {
			glm::vec3 normal;
			lineStream >> normal.x >> normal.y >> normal.z;
			tempNormals.push_back(normal);
		} else if (prefix == "f") {
			char c;
			for (FileIndices triangleIndices[3]; auto& triangleIndex: triangleIndices) {
				lineStream >> triangleIndex.v >> c >> triangleIndex.vt >> c >> triangleIndex.vn;

				triangleIndex.v--;
				triangleIndex.vt--;
				triangleIndex.vn--;
				indices.push_back(addFileIndices(&fileIndices, &triangleIndex));
			}
		}
	}

	size_t size = fileIndices.size();
	vertices.reserve(size);

	for (int i = 0; i < size; i++)
		vertices.emplace_back(tempVertices[fileIndices[i].v], tempUVs[fileIndices[i].vt], tempNormals[fileIndices[i].vn], color);

	setData(vertices, indices);
}

namespace Meshes {
	std::unique_ptr<Mesh<ObjectVertex>> ball;
	std::unique_ptr<Mesh<ObjectVertex>> plane;

	void load() {
		ball = std::make_unique<Mesh<ObjectVertex>>("models/Ball.obj");
		plane = std::make_unique<Mesh<ObjectVertex>>("models/Ground.obj", Color::Boundary);
	}
}