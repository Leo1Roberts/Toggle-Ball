#include "main.h"
#include "Model.h"

void Model::setupBuffers() {
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vertex_buffer);
	glGenBuffers(1, &index_buffer);
}

void Model::sendToGpu() {
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	const Vertex* verticesData = vertices.data();
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), verticesData, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	const Index* indicesData = indices.data();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Index) * indices.size(), indicesData, GL_STATIC_DRAW);
}

Model* ballModel;
Model* planeModel;