#ifndef MODEL_H
#define MODEL_H

#include "Colors.h"
#include "Mesh.h"

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