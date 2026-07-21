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



float UIText::calculateWidth() const {
	auto font = Fonts::get(style.font);
	int length = (int)text.length();
	float width = 0.f;
	for (int i = 0; i < length; i++) {
		char c = text.at(i);
		float scaleFactor = style.fontSize / font->typeface->size;
		if (c == ' ')
			width += font->wordSpacing * scaleFactor;
		else {
			CharBounds bounds = font->typeface->charLocations[c];
			if (bounds.right == 0.f) // char not found
				bounds = font->typeface->charLocations['?'];

			float charWidth = bounds.right - bounds.left;

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
	}

	return width;
}



void UITextRenderer::addText(const UIText* textNode) {
	auto font = Fonts::get(textNode->style.font);
	auto typeface = font->typeface;

	// Draw on texture change
	if (activeTexture != typeface->texture.get()) {
		if (activeTexture != nullptr && !vertices.empty())
			flush(currentProjectionMatrix);
		activeTexture = typeface->texture.get();
	}

	const auto& bounds = textNode->getAbsoluteBounds();
	float xPos = bounds.x;
	float yPos = bounds.y;

	for (char c : textNode->text) {
		float scaleFactor = textNode->style.fontSize / typeface->size;

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
		vertices.emplace_back(glm::vec2(xPos, yPos), glm::vec2(cb.left, cb.top), textNode->style.color);
		// Bottom left
		vertices.emplace_back(glm::vec2(xPos, yPos + finalHeight), glm::vec2(cb.left, cb.bottom), textNode->style.color);
		// Bottom right
		vertices.emplace_back(glm::vec2(xPos + finalWidth, yPos + finalHeight), glm::vec2(cb.right, cb.bottom), textNode->style.color);
		// Top right
		vertices.emplace_back(glm::vec2(xPos + finalWidth, yPos), glm::vec2(cb.right, cb.top), textNode->style.color);

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