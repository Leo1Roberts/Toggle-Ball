
#include "main.h"
#include "Shader.h"
#include "Sizes.h"
#include "MatrixUtilities.h"
#include "Cursor.h"

void CursorVertex::setupLayout() {
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(CursorVertex), (void*)offsetof(CursorVertex, pos));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(CursorVertex), (void*)offsetof(CursorVertex, uv));
	glEnableVertexAttribArray(1);
}

Texture* Cursor::tex;
vec2 Cursor::pos;
float Cursor::angle;
float Cursor::size;
bool Cursor::visible;

std::unique_ptr<Mesh<CursorVertex>> Cursor::mesh;
std::vector<CursorVertex> Cursor::vertices;
std::vector<Index> Cursor::indices;

void Cursor::init() {
	mesh = std::make_unique<Mesh<CursorVertex>>(GL_DYNAMIC_DRAW);

	vertices.reserve(6);
	indices.reserve(6);

	tex = Textures::Cursors::arrow.get();
	angle = 0;
	visible = false;
}

void Cursor::updateProjectionMatrix() {
	mat4 projMat;
	buildOrthographicMatrix(&projMat, 1.0f, RATIO, -1.0f, 1.0f);
	Shaders::cursor->setMat4("uProjection2D", projMat);
}

void Cursor::drawCursor() {
	if (!visible) return;

	updateProjectionMatrix();

	vertices.clear();
	indices.clear();

	vertices.emplace_back(pos + vec2(cos(angle + PI * 0.75f), sin(angle + PI * 0.75f)) * size, vec2(0.0f, 0.0f));
	vertices.emplace_back(pos + vec2(cos(angle + PI * -0.75f), sin(angle + PI * -0.75f)) * size, vec2(0.0f, 1.0f));
	vertices.emplace_back(pos + vec2(cos(angle + PI * -0.25f), sin(angle + PI * -0.25f)) * size, vec2(1.0f, 1.0f));
	vertices.emplace_back(pos + vec2(cos(angle + PI * 0.25f), sin(angle + PI * 0.25f)) * size, vec2(1.0f, 0.0f));

	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(0);
	indices.push_back(2);
	indices.push_back(3);

	Shaders::cursor->use();

	tex->bind(0);

	mesh->setData(vertices, indices);
	mesh->draw();
}
