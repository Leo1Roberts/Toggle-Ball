#include <Mesh.h>
#include <Shader.h>
#include "main.h"
#include "Colors.h"
#include "Sizes.h"
#include "ImageUtils.h"
#include "MatrixUtilities.h"
#include "Text.h"

const int MAX_FONTS = 16;
const int MAX_CHARS = 0x10000;// WARNING: changing this may require changing the type of batchSize
const int MAX_BATCHES = 5;

void CharVertex::setupLayout() {
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(CharVertex), (void*)offsetof(CharVertex, pos));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(CharVertex), (void*)offsetof(CharVertex, uv));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(CharVertex), (void*)offsetof(CharVertex, color));
	glEnableVertexAttribArray(2);
}

struct charInfo {
	char c;
	char _spacer1_;
	char _spacer2_;
	char _spacer3_;
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

#ifdef WINDOWS_VERSION
int Text::loadFace(const std::string& folder, const std::string& name) {
	std::string texturePath = folder + name + ".png";

	auto tex = std::make_unique<Texture>(texturePath, true);

	std::string infoPath = folder + name + ".bin";

	FILE* infoFile;
	fopen_s(&infoFile, infoPath.c_str(), "rb");
	if (infoFile == nullptr) {
		return -1;
	}

	fseek(infoFile, 0, SEEK_END);
	int length = ftell(infoFile) / sizeof(charInfo);
	fseek(infoFile, 0, SEEK_SET);

	charInfo* info = new charInfo[length];

	size_t read = fread(info, sizeof(charInfo), length, infoFile);

	fclose(infoFile);

	CharBounds* charLocations = new CharBounds[CHAR_MAX];

	float digitWidth = 0;
	float maxCharWidth = 0;
	for (int i = 0; i < length; i++) {
		charLocations[info[i].c] = info[i].bounds;

		float charWidth = info[i].bounds.right - info[i].bounds.left;
		if (info[i].c >= '0' && info[i].c <= '9')
			digitWidth = fmaxf(digitWidth, charWidth);
		maxCharWidth = fmaxf(maxCharWidth, charWidth);
	}

	fontFaces[numFonts] = {std::move(tex), charLocations, info[0].bounds.bottom - info[0].bounds.top, digitWidth, maxCharWidth};

	delete[] info;

	return numFonts++;
}
#else

byte Text::loadFace(AAssetManager* assetManager, const std::string& folder, const std::string& name) {
	std::string texturePath = folder + name + ".png";

	auto tex = std::make_unique<Texture>(assetManager, texturePath, true);

	std::string infoPath = folder + name + ".bin";

	AAsset* infoFile = AAssetManager_open(
	        assetManager,
	        infoPath.c_str(),
	        AASSET_MODE_BUFFER);
	if (infoFile == nullptr) {
		return false;
	}

	unsigned int length = AAsset_getLength(infoFile) / sizeof(charInfo);

	charInfo* info = (charInfo*)AAsset_getBuffer(infoFile);

	CharBounds* charLocations = new CharBounds[CHAR_MAX];

	float digitWidth = 0;
	float maxCharWidth = 0;
	for (int i = 0; i < length; i++) {
		charLocations[info[i].c] = info[i].bounds;

		float charWidth = info[i].bounds.right - info[i].bounds.left;
		if (info[i].c >= '0' && info[i].c <= '9')
			digitWidth = fmax(digitWidth, charWidth);
		maxCharWidth = fmax(maxCharWidth, charWidth);
	}

	fontFaces[numFonts] = {std::move(tex), charLocations, info[0].bounds.bottom - info[0].bounds.top, digitWidth, maxCharWidth};

	AAsset_close(infoFile);

	return numFonts++;
}

#endif

void Text::deleteFonts() {
	for (int i = 0; i < numFonts; i++)
		delete[] fontFaces[i].charLocations;
}

int Text::addText(float x, float y, std::string text, const Font& font, float size, const col& textColor) {
	const FontFace& fontFace = fontFaces[font.fontFaceId];
	int length = (int)text.length();
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

float Text::calculateWidth(std::string text, const Font& font, float size) {
	const FontFace& fontFace = fontFaces[font.fontFaceId];
	int length = (int)text.length();
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
	mat4 projMat;
	buildOrthographicMatrix(&projMat, 1.0f, RATIO, -1.0f, 1.0f);
	Shaders::text->setMat4("uProjection2D", projMat);
}
