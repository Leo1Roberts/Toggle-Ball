#ifndef MODEL_H
#define MODEL_H

#include "Colors.h"

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

	GLuint vao;
	GLuint vertex_buffer;
	GLuint index_buffer;

	Model() : vao(0), vertex_buffer(0), index_buffer(0) {
		setupBuffers();
	}

	Model(std::vector<Vertex> iVertices, std::vector<Index> iIndices) :
	    vao(0), vertex_buffer(0), index_buffer(0),
	    vertices(std::move(iVertices)),
	    indices(std::move(iIndices)) {
		setupBuffers();
		sendToGpu();
	}

	~Model() { free(); }

	//Model(const Model& other) :
	//	vao(0),
	//	vertex_buffer(0),
	//	index_buffer(0),
	//	vertices(other.vertices),
	//	indices(other.indices) {
	//	setupBuffers();
	//	sendToGpu();
	//}

	//Model& operator=(const Model& other) {
	//	if (this != &other) {
	//		free();
	//		vertices = other.vertices;
	//		indices = other.indices;
	//		setupBuffers();
	//		sendToGpu();
	//	}
	//	return *this;
	//}

	Model(const Model&) = delete;
	Model& operator=(const Model&) = delete;

	Model(Model&& other) noexcept :
		vao(other.vao),
		vertex_buffer(other.vertex_buffer),
		index_buffer(other.index_buffer),
		vertices(std::move(other.vertices)),
		indices(std::move(other.indices)) {
		other.vao = other.vertex_buffer = other.index_buffer = 0;
	}

	Model& operator=(Model&& other) noexcept {
		if (this != &other) {
			free();
			vao = other.vao;
			vertex_buffer = other.vertex_buffer;
			index_buffer = other.index_buffer;
			vertices = std::move(other.vertices);
			indices = std::move(other.indices);
			other.vao = other.vertex_buffer = other.index_buffer = 0;
		}
		return *this;
	}

	void setupBuffers();

	void sendToGpu();

private:
	
	void free() {
		if (vao) glDeleteVertexArrays(1, &vao);
		if (vertex_buffer) glDeleteBuffers(1, &vertex_buffer);
		if (index_buffer) glDeleteBuffers(1, &index_buffer);
		vao = vertex_buffer = index_buffer = 0;
	}
};

extern Model* ballModel;
extern Model* planeModel;

#endif // MODEL_H