#include "ui/UIText.h"
#include "ui/Font.h"
#include "opengl/Shader.h"
#include "opengl/Texture.h"
#include "ui/UIManager.h"


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



void UIText::updateTextLayout() {
	textLayout.reset();

	if (text.empty())
		textLayout.cursorPositions.emplace_back(0.f);

	auto font = Fonts::get(textStyle.font);
	auto typeface = font->typeface;
	float scaleFactor = textStyle.fontSize / typeface->size;

	textLayout.cursorPositions.resize(text.length() + 1);
	textLayout.charAdvances.resize(text.length());

	struct LineInfo {
		int start;
		int end;
		float width;
	};
	std::vector<LineInfo> lines;

	int currentLineStart = 0;
	float currentLineWidth = 0.f;

	for (int i = 0; i < text.length(); i++) {
		char c = text[i];
		if (c == '\n') {
			float lineWidth = currentLineWidth;
			if (i > currentLineStart && text[i - 1] != ' ')
				lineWidth -= font->charSpacing * scaleFactor;

			lines.push_back({currentLineStart, i, std::max(0.f, lineWidth)});
			currentLineStart = i + 1;
			currentLineWidth = 0.f;
			continue;
		}

		float advance = 0.f;
		if (c == ' ') {
			advance = font->wordSpacing * scaleFactor;
		} else {
			CharBounds cb = typeface->charLocations[c];
			if (cb.right == 0.f) cb = typeface->charLocations['?'];

			float charWidth = cb.right - cb.left;
			float padding = 0.f;
			if (font->monoSpaced)
				padding = (typeface->maxCharWidth - charWidth) / 2.f;
			else if (c >= '0' && c <= '9')
				padding = (typeface->digitWidth - charWidth) / 2.f;

			advance = (charWidth + 2.f * padding + font->charSpacing) * scaleFactor;
		}
		currentLineWidth += advance;
	}

	float finalLineWidth = currentLineWidth;
	if ((int)text.length() > currentLineStart && text.back() != ' ')
		finalLineWidth -= font->charSpacing * scaleFactor;
	lines.push_back({currentLineStart, (int)text.length(), std::max(0.f, finalLineWidth)});
	
	float totalHeight = (float)lines.size() * textStyle.fontSize +
						(float)(lines.size() - 1) * font->lineSpacing * scaleFactor;
	textLayout.totalSize = {0.f, totalHeight};

	const auto& bounds = getAbsoluteBounds();
	
	float startY;
	switch (textStyle.alignVertical) {
	case TextAlignVertical::Top:
		startY = 0.f;
		break;
	case TextAlignVertical::Middle:
		startY = (bounds.height - totalHeight) / 2.f;
		break;
	case TextAlignVertical::Bottom:
		startY = bounds.height - totalHeight;
		break;
	}

	float currentY = startY;
	for (const auto& line : lines) {
		textLayout.totalSize.x = std::max(textLayout.totalSize.x, line.width);

		float currentX;
		switch (textStyle.alignHorizontal) {
		case TextAlignHorizontal::Left:
			currentX = 0.f;
			break;
		case TextAlignHorizontal::Centre:
			currentX = (bounds.width - line.width) * 0.5f;
			break;
		case TextAlignHorizontal::Right:
			currentX = bounds.width - line.width;
			break;
		}

		if (line.start == line.end) {
			textLayout.cursorPositions[line.start] = glm::vec2(currentX, currentY);
			currentY += textStyle.fontSize + font->lineSpacing * scaleFactor;
			continue;
		}

		struct VisualBounds { float left; float right; };
		std::vector<VisualBounds> visualBounds(line.end - line.start);

		for (int i = line.start; i < line.end; i++) {
			char c = text[i];
			int localIdx = i - line.start;
			float advance = 0.f;

			if (c == ' ') {
				advance = font->wordSpacing * scaleFactor;
				visualBounds[localIdx].left = currentX;
				visualBounds[localIdx].right = currentX + advance;
			} else {
				CharBounds cb = typeface->charLocations[c];
				if (cb.right == 0.f) cb = typeface->charLocations['?'];

				float charWidth = cb.right - cb.left;
				float padding = 0.f;
				if (font->monoSpaced)
					padding = (typeface->maxCharWidth - charWidth) / 2.f;
				else if (c >= '0' && c <= '9')
					padding = (typeface->digitWidth - charWidth) / 2.f;

				advance = (charWidth + 2.f * padding + font->charSpacing) * scaleFactor;

				float glyphX = currentX + padding * scaleFactor;
				float glyphWidth = charWidth * scaleFactor;
				
				visualBounds[localIdx].left = currentX;
				visualBounds[localIdx].right = currentX + (charWidth + 2.f * padding) * scaleFactor;

				TextGlyph glyph{};
				glyph.pos = {glyphX, currentY};
				glyph.size = {glyphWidth, (cb.bottom - cb.top) * scaleFactor};
				glyph.uvLeftTop = {cb.left, cb.top};
				glyph.uvRightBottom = {cb.right, cb.bottom};
				textLayout.glyphs.push_back(glyph);
			}

			currentX += advance;
		}

		float edgePadding = font->charSpacing * scaleFactor / 2.f;
		textLayout.cursorPositions[line.start] = glm::vec2(visualBounds[0].left - edgePadding, currentY);

		for (int i = line.start + 1; i < line.end; i++) {
			int prevLocal = i - 1 - line.start;
			int currLocal = i - line.start;

			float midX = (visualBounds[prevLocal].right + visualBounds[currLocal].left) / 2.f;
			textLayout.cursorPositions[i] = glm::vec2(midX, currentY);
			textLayout.charAdvances[i - 1] = midX - textLayout.cursorPositions[i - 1].x;
		}

		int lastLocal = line.end - 1 - line.start;
		textLayout.cursorPositions[line.end] = glm::vec2(visualBounds[lastLocal].right + edgePadding, currentY);
		textLayout.charAdvances[line.end - 1] = textLayout.cursorPositions[line.end].x - textLayout.cursorPositions[line.end - 1].x;

		if (line.end < (int)text.length())
			textLayout.charAdvances[line.end] = 0.f; // \n character

		currentY += textStyle.fontSize + font->lineSpacing * scaleFactor;
	}
}


int UIText::getIndexAtPosition(glm::vec2 localPos, bool cursor) const {
	if (textLayout.cursorPositions.empty()) return 0;

	float lineHeight = textStyle.fontSize;
	if (auto font = Fonts::get(textStyle.font))
		lineHeight += font->lineSpacing * (textStyle.fontSize / font->typeface->size);

	int bestIndex = 0;
	float minLineDist = 999999.f;
	float minXDist = 999999.f;

	int maxIndex = cursor ? (int)text.length() : std::max(0, (int)text.length() - 1);

	for (int i = 0; i <= maxIndex; i++) {
		glm::vec2 cursorPos = textLayout.cursorPositions[i];
		float advance = (i < text.length()) ? textLayout.charAdvances[i] : 0.f;

		float lineCenterY = cursorPos.y + lineHeight * 0.5f;
		float lineDist = std::abs(localPos.y - lineCenterY);

		if (lineDist < minLineDist - 1.0f) { // Changed line
			minLineDist = lineDist;
			minXDist = 999999.f;
		}

		if (std::abs(lineDist - minLineDist) < 1.0f) { // On the closest line
			float targetX = cursor ? cursorPos.x : (cursorPos.x + advance * 0.5f);
			float xDist = std::abs(localPos.x - targetX);

			if (xDist < minXDist) {
				minXDist = xDist;
				bestIndex = i;
			}
		}
	}
	return bestIndex;
}


std::vector<Rectangle> UIText::getHighlightRects(int start, int end) const {
	std::vector<Rectangle> rects;
	if (start == end) return rects;

	float lineHeight = textStyle.fontSize;
	float currentX = textLayout.cursorPositions[start].x;
	float currentY = textLayout.cursorPositions[start].y;
	float currentWidth = 0.f;

	for (int i = start; i < end; i++) {
		if (textLayout.cursorPositions[i].y != currentY) { // Start a new rectangle if new line reached
			rects.push_back({.x = currentX, .y = currentY, .width = currentWidth, .height = lineHeight});
			currentX = textLayout.cursorPositions[i].x;
			currentY = textLayout.cursorPositions[i].y;
			currentWidth = 0.f;
		}
		currentWidth += textLayout.charAdvances[i];
	}

	rects.push_back({.x = currentX, .y = currentY, .width = currentWidth, .height = lineHeight});
	return rects;
}


void UITextRenderer::addText(const UIText* textNode) {
	auto font = Fonts::get(textNode->textStyle.font);
	auto typeface = font->typeface;

	if (activeTexture != typeface->texture.get()) {
		if (activeTexture != nullptr && !vertices.empty())
			flush(currentProjectionMatrix);
		activeTexture = typeface->texture.get();
	}

	const TextLayout& layout = textNode->getTextLayout();
	const auto& bounds = textNode->getAbsoluteBounds();

	for (const auto& glyph : layout.glyphs) {
		float x = bounds.x + glyph.pos.x;
		float y = bounds.y + glyph.pos.y;
		float width = glyph.size.x;
		float height = glyph.size.y;

		Index base = vertices.size();

		vertices.emplace_back(glm::vec2(x, y), glyph.uvLeftTop, textNode->textStyle.color);
		vertices.emplace_back(glm::vec2(x, y + height), glm::vec2(glyph.uvLeftTop.x, glyph.uvRightBottom.y), textNode->textStyle.color);
		vertices.emplace_back(glm::vec2(x + width, y + height), glyph.uvRightBottom, textNode->textStyle.color);
		vertices.emplace_back(glm::vec2(x + width, y), glm::vec2(glyph.uvRightBottom.x, glyph.uvLeftTop.y), textNode->textStyle.color);

		indices.push_back(base); indices.push_back(base + 1); indices.push_back(base + 2);
		indices.push_back(base); indices.push_back(base + 2); indices.push_back(base + 3);
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