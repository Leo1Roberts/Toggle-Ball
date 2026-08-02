#include "UIText.h"
#include "Font.h"
#include "Shader.h"
#include "Texture.h"
#include "UIManager.h"


void CharVertex::setupLayout() {
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(CharVertex), (void*)offsetof(CharVertex, position));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(CharVertex), (void*)offsetof(CharVertex, uv));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(CharVertex), (void*)offsetof(CharVertex, color));
	glEnableVertexAttribArray(2);
}



void UIText::submitRender(UIManager& manager) {
	manager.submitText(this);
}



glm::vec2 UIText::measure(int from, int to) const {
	if (text.empty()) return {0.f, 0.f};
	if (to < 0)
		to = (int)text.length();

	auto font = Fonts::get(textStyle.font);
	auto typeface = font->typeface;

	float width = 0.f;
	float scaleFactor = textStyle.fontSize / typeface->size;

	for (int i = from; i < to; i++) {
		char c = text[i];
		if (c == ' ') {
			width += font->wordSpacing * scaleFactor;
			continue;
		}

		CharBounds cb = typeface->charLocations[c];
		if (cb.right == 0.f) cb = typeface->charLocations['?'];

		float charWidth = cb.right - cb.left;

		if (font->monoSpaced) {
			float padding = (font->typeface->maxCharWidth - charWidth) / 2.f;
			width += padding * scaleFactor;
			charWidth += padding;
		} else if (c >= '0' && c <= '9') {
			float padding = (font->typeface->digitWidth - charWidth) / 2.f;
			width += padding * scaleFactor;
			charWidth += padding;
		}

		width += (charWidth + font->charSpacing) * scaleFactor;
	}

	float height = textStyle.fontSize;

	return glm::vec2(width, height);
}



void UITextRenderer::addText(const UIText* textNode) {
	auto font = Fonts::get(textNode->textStyle.font);
	auto typeface = font->typeface;

	// Draw on texture change
	if (activeTexture != typeface->texture.get()) {
		if (activeTexture != nullptr && !vertices.empty())
			flush(currentProjectionMatrix);
		activeTexture = typeface->texture.get();
	}

	const auto& bounds = textNode->getAbsoluteBounds();

	glm::vec2 textSize = textNode->measure(0);

	float xPos = bounds.x;
	switch (textNode->textStyle.alignHorizontal) {
	case TextAlignHorizontal::Centre:
		xPos += (bounds.width - textSize.x) * 0.5f;
		break;
	case TextAlignHorizontal::Right:
		xPos += (bounds.width - textSize.x);
		break;
	case TextAlignHorizontal::Left:
	default:
		break;
	}

	float yPos = bounds.y;
	switch (textNode->textStyle.alignVertical) {
	case TextAlignVertical::Middle:
		yPos += (bounds.height - textSize.y) * 0.5f;
		break;
	case TextAlignVertical::Bottom:
		yPos += (bounds.height - textSize.y);
		break;
	case TextAlignVertical::Top:
	default:
		break;
	}

	for (char c : textNode->text) {
		float scaleFactor = textNode->textStyle.fontSize / typeface->size;

		if (c == ' ') {
			xPos += font->wordSpacing * scaleFactor;
			continue;
		}

		CharBounds cb = typeface->charLocations[c];
		if (cb.right == 0.f)
			cb = typeface->charLocations['?'];

		float width = cb.right - cb.left;

		if (font->monoSpaced) {
			float padding = (typeface->maxCharWidth - width) / 2.f;
			xPos += padding * scaleFactor;
			width += padding;
		} else if (c >= '0' && c <= '9') {
			float padding = (typeface->digitWidth - width) / 2.f;
			xPos += padding * scaleFactor;
			width += padding;
		}

		float finalWidth = (cb.right - cb.left) * scaleFactor;
		float finalHeight = (cb.bottom - cb.top) * scaleFactor;

		Index base = vertices.size();

		// Top left
		vertices.emplace_back(glm::vec2(xPos, yPos), glm::vec2(cb.left, cb.top), textNode->textStyle.color);
		// Bottom left
		vertices.emplace_back(glm::vec2(xPos, yPos + finalHeight), glm::vec2(cb.left, cb.bottom), textNode->textStyle.color);
		// Bottom right
		vertices.emplace_back(glm::vec2(xPos + finalWidth, yPos + finalHeight), glm::vec2(cb.right, cb.bottom), textNode->textStyle.color);
		// Top right
		vertices.emplace_back(glm::vec2(xPos + finalWidth, yPos), glm::vec2(cb.right, cb.top), textNode->textStyle.color);

		indices.push_back(base); indices.push_back(base + 1); indices.push_back(base + 2);
		indices.push_back(base); indices.push_back(base + 2); indices.push_back(base + 3);

		xPos += (width + font->charSpacing) * scaleFactor;
	}
}


void UITextRenderer::flush(const glm::mat4& projectionMatrix) {
	if (vertices.empty() || !activeTexture) return;

	Shaders::text->use();
	Shaders::text->setMat4("uProjection2D", projectionMatrix);

	activeTexture->bind(0);

	mesh->setData(vertices, indices);
	mesh->draw();

	vertices.clear();
	indices.clear();
}