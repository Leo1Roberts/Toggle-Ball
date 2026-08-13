#ifndef UI_TEXT_H
#define UI_TEXT_H

#include <utility>

#include "ui/UINode.h"
#include "opengl/Mesh.h"
#include "ui/UIStyle.h"


struct CharVertex {
	glm::vec2 position;
	glm::vec2 uv;
	col color;

	CharVertex(glm::vec2 p, glm::vec2 u, col c) : position(p), uv(u), color(c) {}

	static void setupLayout();
};

struct TextGlyph {
	glm::vec2 pos; // Relative position within UITextNode
	glm::vec2 size;
	glm::vec2 uvLeftTop;
	glm::vec2 uvRightBottom;
};

struct TextLayout {
	glm::vec2 totalSize{};
	// For UI interation
	std::vector<glm::vec2> cursorPositions; // Absolute position
	std::vector<float> charAdvances;
	// For rendering
	std::vector<TextGlyph> glyphs;

	void reset() {
		totalSize = glm::vec2(0.f);
		cursorPositions.clear();
		charAdvances.clear();
		glyphs.clear();
	}
};


struct Font;

class UIText : public UINode {
public:
	explicit UIText(std::string text, const TextStyle& style = {})
		: textStyle(style), text(std::move(text)) {
		setHitTestable(false);
		setHitTestableChildren(false);
		updateTextLayout();
	}

	void updateBounds(Rectangle parentBounds) override {
		UINode::updateBounds(parentBounds);
		updateTextLayout();
	}

	void submitRender(UIManager& manager) override;

	void updateTextLayout();

	[[nodiscard]] glm::vec2 getCursorPosition(int index) const { return textLayout.cursorPositions[index]; }
	// Returns the index of the closest (cursor ? cursor : character)
	[[nodiscard]] int getIndexAtPosition(glm::vec2 localPos, bool cursor) const;
	[[nodiscard]] std::vector<Rectangle> getHighlightRects(int start, int end) const;

	TextStyle textStyle;

	void setText(const std::string& newText) {
		text = newText;
		updateTextLayout();
	}

	[[nodiscard]] const std::string& getText() const { return text; }
	[[nodiscard]] const TextLayout& getTextLayout() const { return textLayout; }

private:
	std::string text;
	TextLayout textLayout;
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
