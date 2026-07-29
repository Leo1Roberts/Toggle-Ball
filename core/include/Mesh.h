#ifndef MESH_H
#define MESH_H

#include "main.h"
#include "Colors.h"
#include "GLUtilities.h"

#include <glm/glm.hpp>
#include <memory>
#include <vector>

using Index = unsigned short; // Needs to match glDrawElements 'type' argument

struct ObjectVertex {
	glm::vec3 position{};
	glm::vec2 uv{};
	glm::vec3 normal{};
	col color;

	ObjectVertex() = default;
	ObjectVertex(glm::vec3 position, glm::vec2 uv, glm::vec3 normal, col color = Color::White) : position(position), uv(uv), normal(normal), color(color) {}

	static void setupLayout();
};

template <typename TVertex>
class Mesh {
public:
	Mesh(GLenum usage) : usage(usage) {
		glGenVertexArrays(1, VAO.ptr());
		glGenBuffers(1, VBO.ptr());
		glGenBuffers(1, EBO.ptr());

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

		TVertex::setupLayout();

		glBindVertexArray(0);
	}

	Mesh(const std::vector<TVertex>& vertices, const std::vector<Index>& indices, GLenum usage = GL_STATIC_DRAW) :
	    Mesh(usage) { setData(vertices, indices); }

	explicit Mesh(const std::string& path, col color = Color::White);

	void setData(const std::vector<TVertex>& vertices, const std::vector<Index>& indices) {
		indexCount = static_cast<GLsizei>(indices.size());

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(TVertex), vertices.data(), usage);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indices.size() * sizeof(Index)), indices.data(), usage);
	}

	void draw() const {
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, nullptr); // 'type' argument needs to match Index
		glBindVertexArray(0);
	}

private:
	GLVertexArray VAO;
	GLBuffer VBO;
	GLBuffer EBO;

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
