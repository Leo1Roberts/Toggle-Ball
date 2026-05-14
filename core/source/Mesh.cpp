#include "main.h"
#include "Mesh.h"

void Vertex3D::setupLayout() {
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)offsetof(Vertex3D, position));
	CHECK_ERROR();
	glEnableVertexAttribArray(0);
	CHECK_ERROR();

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)offsetof(Vertex3D, uv));
	CHECK_ERROR();
	glEnableVertexAttribArray(1);
	CHECK_ERROR();

	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)offsetof(Vertex3D, normal));
	CHECK_ERROR();
	glEnableVertexAttribArray(2);
	CHECK_ERROR();

	glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex3D), (void*)offsetof(Vertex3D, color));
	CHECK_ERROR();
	glEnableVertexAttribArray(3);
	CHECK_ERROR();
}