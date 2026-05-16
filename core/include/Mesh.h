#ifndef MESH_H
#define MESH_H

#include "main.h"
#include "Colors.h"

using Index = unsigned short; // Needs to match glDrawElements 'type' argument

struct ObjectVertex {
	vec3 position;
	vec2 uv;
	vec3 normal;
	col color;

	ObjectVertex() = default;
	ObjectVertex(vec3 position, vec2 uv, vec3 normal, col color) : position(position), uv(uv), normal(normal), color(color) {}

	static void setupLayout();
};

template <typename TVertex>
class Mesh {
public:
	Mesh(GLenum usage) : usage(usage) {
		glGenVertexArrays(1, vao.ptr());
		glGenBuffers(1, vbo.ptr());
		glGenBuffers(1, ebo.ptr());

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

		TVertex::setupLayout();

		glBindVertexArray(0);
	}

	Mesh(const std::vector<TVertex>& vertices, const std::vector<Index>& indices, GLenum usage = GL_STATIC_DRAW) :
	    Mesh(usage) { setData(vertices, indices); }

#ifdef WINDOWS_VERSION
	Mesh(const std::string& path, col color = WHITE);
#else
	Mesh(AAssetManager* assetManager, const std::string& path, col color = WHITE);
#endif

	void setData(const std::vector<TVertex>& vertices, const std::vector<Index>& indices) {
		indexCount = static_cast<GLsizei>(indices.size());

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(TVertex), vertices.data(), usage);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(Index), indices.data(), usage);
	}

	void draw() const {
		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, 0); // 'type' argument needs to match Index
		glBindVertexArray(0);
	}

private:
	GLVertexArray vao;
	GLBuffer vbo;
	GLBuffer ebo;

	GLsizei indexCount = 0;
	GLenum usage; // GL_STATIC_DRAW or GL_DYNAMIC_DRAW
};

template <>
#ifdef WINDOWS_VERSION
Mesh<ObjectVertex>::Mesh(const std::string& path, col color);
#else
Mesh<ObjectVertex>::Mesh(AAssetManager* assetManager, const std::string& path, col color);
#endif

namespace Meshes {
	extern std::unique_ptr<Mesh<ObjectVertex>> ball;
	extern std::unique_ptr<Mesh<ObjectVertex>> plane;

#ifdef WINDOWS_VERSION
	void load();
#else
	void load(AAssetManager* assetManager);
#endif
}

#endif // MESH_H
