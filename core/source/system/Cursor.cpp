#include "system/Cursor.h"

#include "opengl/Shader.h"


void CursorVertex::setupLayout() {
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(CursorVertex), (void*)offsetof(CursorVertex, pos));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(CursorVertex), (void*)offsetof(CursorVertex, uv));
	glEnableVertexAttribArray(1);
}


std::unique_ptr<Texture> Cursor::textures[(int)Style::COUNT];

void Cursor::loadTextures() {
#if defined(CUSTOM_CURSORS)
	textures[(int)Style::Arrow           ] = std::make_unique<Texture>("cursors/arrow.png"           , false, true);
	textures[(int)Style::PointingHand    ] = std::make_unique<Texture>("cursors/hand.png"            , false, true);
	textures[(int)Style::Text            ] = std::make_unique<Texture>("cursors/text.png"            , false, true);
	textures[(int)Style::HorizontalResize] = std::make_unique<Texture>("cursors/horizontalResize.png", false, true);
	textures[(int)Style::VerticalResize  ] = std::make_unique<Texture>("cursors/verticalResize.png"  , false, true);
#endif
	textures[(int)Style::DynamicResize   ] = std::make_unique<Texture>("cursors/resize.png"          , false, true);
}


std::vector<CursorVertex> Cursor::vertices;
std::vector<Index> Cursor::indices;
std::unique_ptr<Mesh<CursorVertex>> Cursor::mesh;

void Cursor::drawDynamic(float size, glm::vec2 pos, glm::vec2 screenSize) const {
	if (!mesh)
		mesh = std::make_unique<Mesh<CursorVertex>>(GL_DYNAMIC_DRAW);

	Shaders::cursor->use();
	Shaders::cursor->setMat4("uProjection2D", glm::ortho(0.f, screenSize.x, screenSize.y, 0.f));

	vertices.clear();
	indices.clear();

	float halfSize = size * 0.5f;
	float cosA = std::cos(angle);
	float sinA = std::sin(angle);

	// Scaled basis vectors for the rotated axes
	glm::vec2 right(cosA * halfSize, sinA * halfSize);
	glm::vec2 down(-sinA * halfSize, cosA * halfSize);

	// Quad vertices centered at 'pos' with dimensions size x size
	vertices.emplace_back(pos - right - down, glm::vec2(0.f, 0.f)); // Top-Left
	vertices.emplace_back(pos - right + down, glm::vec2(0.f, 1.f)); // Bottom-Left
	vertices.emplace_back(pos + right + down, glm::vec2(1.f, 1.f)); // Bottom-Right
	vertices.emplace_back(pos + right - down, glm::vec2(1.f, 0.f)); // Top-Right

	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(0);
	indices.push_back(2);
	indices.push_back(3);

	getTexture()->bind(0);

	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);

	mesh->setData(vertices, indices);
	mesh->draw();

	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
}