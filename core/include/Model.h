#ifndef MODEL_H
#define MODEL_H

#include "Colors.h"

struct BufferDeleter {
	void operator()(GLsizei n, const GLuint* ids) const { glDeleteBuffers(n, ids); }
};
struct VAODeleter {
	void operator()(GLsizei n, const GLuint* ids) const { glDeleteVertexArrays(n, ids); }
};

template <typename T, typename Deleter>
class GLHandle {
public:
	GLHandle() = default;
	explicit GLHandle(T id) : id(id) {}

	~GLHandle() { if (id) Deleter{}(1, &id); }

	GLHandle(const GLHandle&) = delete;
	GLHandle& operator=(const GLHandle&) = delete;

	GLHandle(GLHandle&& other) noexcept : id(other.id) { other.id = 0; }
	GLHandle& operator=(GLHandle&& other) noexcept {
		if (this != &other) {
			if (id) Deleter{}(1, &id);
			id = other.id;
			other.id = 0;
		}
		return *this;
	}

	operator T() const { return id; }
	T* ptr() { return &id; }

private:
	T id = 0;
};

using GLBuffer = GLHandle<GLuint, BufferDeleter>;
using GLVertexArray = GLHandle<GLuint, VAODeleter>;

struct Vertex {
	constexpr Vertex(const vec3& iPosition, const vec2& iUv, const vec3& iNormal, const col& iColor) : position(iPosition),
	                                                                                                   uv(iUv),
	                                                                                                   normal(iNormal),
	                                                                                                   color(iColor) {}

	Vertex() : position({0, 0, 0}),
	           uv({0, 0}),
	           normal({0, 0, 0}),
	           color(WHITE) {}

	vec3 position;
	vec2 uv;
	vec3 normal;
	col color;
};

typedef uint16_t Index;

struct Model {
	std::vector<Vertex> vertices;
	std::vector<Index> indices;

	GLVertexArray vao;
	GLBuffer vertex_buffer;
	GLBuffer index_buffer;

	Model() { setupBuffers(); }

	Model(std::vector<Vertex> iVertices, std::vector<Index> iIndices) :
	    vertices(std::move(iVertices)),
	    indices(std::move(iIndices)) {
		setupBuffers();
		sendToGpu();
	}

	void setupBuffers();
	void sendToGpu();
};

extern Model* ballModel;
extern Model* planeModel;

#endif // MODEL_H