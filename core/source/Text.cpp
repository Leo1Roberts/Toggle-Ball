#include <Mesh.h>
#include <Shader.h>
#include "main.h"
#include "Colors.h"
#include "Sizes.h"
#include "MatrixUtilities.h"
#include "Text.h"

#include "AssetManager.h"

constexpr int MAX_FONTS = 16;
constexpr int MAX_CHARS = 0x10000; // WARNING: changing this may require changing the type of batchSize
constexpr int MAX_BATCHES = 5;

void CharVertex::setupLayout() {
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(CharVertex), reinterpret_cast<void*>(offsetof(CharVertex, pos)));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(CharVertex), reinterpret_cast<void*>(offsetof(CharVertex, uv)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(CharVertex), reinterpret_cast<void*>(offsetof(CharVertex, color)));
	glEnableVertexAttribArray(2);
}

struct charInfo {
	char c;
	[[maybe_unused]] char _spacer1_;
	[[maybe_unused]] char _spacer2_;
	[[maybe_unused]] char _spacer3_;
	CharBounds bounds;
};

FontFace Text::fontFaces[MAX_FONTS];
int Text::numFonts;

CharToDraw Text::charsToDraw[MAX_CHARS];
int Text::numChars;
int Text::numCharsDrawn;
byte Text::batchToFill;
unsigned short Text::batchSize[MAX_BATCHES];

std::unique_ptr<Mesh<CharVertex>> Text::mesh;
std::vector<CharVertex> Text::vertices;
std::vector<Index> Text::indices;


void Text::init() {
	mesh = std::make_unique<Mesh<CharVertex>>(GL_DYNAMIC_DRAW);

	vertices.reserve(MAX_CHARS * 4);
	indices.reserve(MAX_CHARS * 6);
}

int Text::loadFace(const std::string& name) {
	std::string path = "fonts/" + name;
	std::string texturePath = path + ".png";
	std::string infoPath = path + ".bin";

	auto tex = std::make_unique<Texture>(texturePath, true);

	FontFace* ff = &fontFaces[numFonts];

	ff->texture = std::move(tex);

	std::vector<byte> buffer = AssetManager::loadAssetToBuffer(infoPath);
	const auto* info = reinterpret_cast<const charInfo*>(buffer.data());
	size_t length = buffer.size() / sizeof(charInfo);

	float digitWidth = 0;
	float maxCharWidth = 0;
	for (int i = 0; i < length; i++) {
		ff->charLocations[info[i].c] = info[i].bounds;

		float charWidth = info[i].bounds.right - info[i].bounds.left;
		if (info[i].c >= '0' && info[i].c <= '9')
			digitWidth = std::max(digitWidth, charWidth);
		maxCharWidth = std::max(maxCharWidth, charWidth);
	}

	ff->size = info[0].bounds.bottom - info[0].bounds.top;
	ff->digitWidth = digitWidth;
	ff->maxCharWidth = maxCharWidth;

	return numFonts++;
}

int Text::addText(float x, float y, const std::string& text, const Font& font, float size, const col& textColor) {
	const FontFace& fontFace = fontFaces[font.fontFaceId];
	int length = static_cast<int>(text.length());
	float xPos = x;
	for (int i = 0; i < length; i++) {
		if (numChars == MAX_CHARS) return i;
		char c = text.at(i);
		float scaleFactor = size / fontFace.size;
		if (c == ' ') {
			xPos += font.wordSpacing * scaleFactor;
		} else {
			CharBounds bounds = fontFace.charLocations[c];
			if (bounds.right == 0)// char not found
				bounds = fontFace.charLocations['?'];

			float width = bounds.right - bounds.left;
			if (font.monoSpaced) {
				float padding = (fontFace.maxCharWidth - width) / 2;
				xPos += padding * scaleFactor;
				width += padding;
			} else if (c >= '0' && c <= '9') {
				float padding = (fontFace.digitWidth - width) / 2;
				xPos += padding * scaleFactor;
				width += padding;
			}
			charsToDraw[numChars++] = {{xPos, y}, fontFace.texture.get(), bounds, textColor, scaleFactor};
			batchSize[batchToFill]++;
			xPos += (width + font.charSpacing) * scaleFactor;
		}
	}

	return length;
}

float Text::calculateWidth(const std::string& text, const Font& font, float size) {
	const FontFace& fontFace = fontFaces[font.fontFaceId];
	int length = static_cast<int>(text.length());
	float width = 0;
	for (int i = 0; i < length; i++) {
		char c = text.at(i);
		float scaleFactor = size / fontFace.size;
		if (c == ' ') {
			width += font.wordSpacing * scaleFactor;
		} else {
			CharBounds bounds = fontFace.charLocations[c];
			if (bounds.right == 0)// char not found
				bounds = fontFace.charLocations['?'];
			float charWidth = bounds.right - bounds.left;
			if (font.monoSpaced) {
				float padding = (fontFace.maxCharWidth - charWidth) / 2;
				width += padding * scaleFactor;
				charWidth += padding;
			} else if (c >= '0' && c <= '9') {
				float padding = (fontFace.digitWidth - charWidth) / 2;
				width += padding * scaleFactor;
				charWidth += padding;
			}
			width += (charWidth + font.charSpacing) * scaleFactor;
		}
	}

	return width;
}

void Text::drawText(byte batch) {
	if (batch == 0)
		batchToFill = 0;

	const int nextBatchStart = numCharsDrawn + batchSize[batch];

	for (int i = 0; i < numFonts; i++) {
		Texture* tex = fontFaces[i].texture.get();

		vertices.clear();
		indices.clear();

		int charNo = 0;
		for (int j = numCharsDrawn; j < nextBatchStart; j++) {
			CharToDraw& ctd = charsToDraw[j];
			if (ctd.texture == tex) {
				CharBounds bounds = ctd.bounds;
				vec2 pos = ctd.pos;

				float width = (bounds.right - bounds.left) * ctd.scaleFactor;
				float height = (bounds.bottom - bounds.top) * ctd.scaleFactor;

				vertices.emplace_back(vec2(pos.x, pos.y), vec2(bounds.left, bounds.top), ctd.color);
				vertices.emplace_back(vec2(pos.x, pos.y - height), vec2(bounds.left, bounds.bottom), ctd.color);
				vertices.emplace_back(vec2(pos.x + width, pos.y - height), vec2(bounds.right, bounds.bottom), ctd.color);
				vertices.emplace_back(vec2(pos.x + width, pos.y), vec2(bounds.right, bounds.top), ctd.color);

				indices.push_back(charNo * 4);
				indices.push_back(charNo * 4 + 1);
				indices.push_back(charNo * 4 + 2);
				indices.push_back(charNo * 4);
				indices.push_back(charNo * 4 + 2);
				indices.push_back(charNo * 4 + 3);

				charNo++;
			}
		}

		Shaders::text->use();

		tex->bind(0);

		mesh->setData(vertices, indices);
		mesh->draw();
	}

	numCharsDrawn = nextBatchStart;
	batchSize[batch] = 0;
}

void Text::updateProjectionMatrix() {
	mat4 projMat{};
	buildOrthographicMatrix(&projMat, 1.0f, RATIO, -1.0f, 1.0f);
	Shaders::text->setMat4("uProjection2D", projMat);
}
