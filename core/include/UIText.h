#ifndef UI_TEXT_H
#define UI_TEXT_H

#include "UINode.h"
#include "Mesh.h"


struct CharVertex {
	glm::vec2 position;
	glm::vec2 uv;
	col color;

	CharVertex(glm::vec2 p, glm::vec2 u, col c) : position(p), uv(u), color(c) {}

	static void setupLayout();
};


struct Font;

class UIText : public UINode {
public:
	void submitRender(UIManager& manager) override;

	[[nodiscard]] float calculateWidth() const;

	std::string text;
	const Font* font;
	float size;
	col color;
};


class Texture;

class UITextRenderer : public IUIRenderer {
public:
	void begin(const glm::mat4& projectionMatrix) { currentProjectionMatrix = projectionMatrix; }

	void addText(const UIText* textNode);

	void flush(const glm::mat4& projectionMatrix) override;

private:
	std::vector<CharVertex> vertices;
	std::vector<Index> indices;
	std::unique_ptr<Mesh<CharVertex>> mesh = std::make_unique<Mesh<CharVertex>>(GL_DYNAMIC_DRAW);

	const Texture* activeTexture = nullptr;
	glm::mat4 currentProjectionMatrix;
};


#endif // UI_TEXT_H
