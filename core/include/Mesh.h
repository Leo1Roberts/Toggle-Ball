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
	ObjectVertex(vec3 position, vec2 uv, vec3 normal, col color = WHITE) : position(position), uv(uv), normal(normal), color(color) {}

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

	Mesh(const std::string& path, col color = WHITE);

	void setData(const std::vector<TVertex>& vertices, const std::vector<Index>& indices) {
		indexCount = static_cast<GLsizei>(indices.size());

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(TVertex), vertices.data(), usage);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indices.size() * sizeof(Index)), indices.data(), usage);
	}

	void draw() const {
		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, nullptr); // 'type' argument needs to match Index
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
Mesh<ObjectVertex>::Mesh(const std::string& path, col color);

namespace Meshes {
	extern std::unique_ptr<Mesh<ObjectVertex>> ball;
	extern std::unique_ptr<Mesh<ObjectVertex>> plane;

	void load();
}

#endif // MESH_H
