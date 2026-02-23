#include "main.h"
#include "Colors.h"
#include "Sizes.h"
#include "ImageUtils.h"
#include "MatrixUtilities.h"
#include "Text.h"

const int MAX_FONTS = 16;
const int MAX_CHARS = 0x10000;// WARNING: changing this may require changing the type of batchSize
const int MAX_BATCHES = 5;

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

GLuint Text::program;
GLint Text::position;
GLint Text::uv;
GLint Text::color;
GLint Text::projectionMatrix;

GLuint Text::vao;
GLuint Text::vertex_buffer;
GLuint Text::index_buffer;
CharVertex Text::vertices[MAX_CHARS * 4];
Index Text::indices[MAX_CHARS * 6];

#ifdef WINDOWS_VERSION
int Text::loadFace(const std::string& folder, const std::string& name) {
	std::string texturePath = folder + name + ".png";

	TextureAsset* tex = TextureAsset::loadAsset(texturePath, true);

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

	fontFaces[numFonts] = {tex, charLocations, info[0].bounds.bottom - info[0].bounds.top, digitWidth, maxCharWidth};

	delete[] info;

	return numFonts++;
}
#else

byte Text::loadFace(AAssetManager* assetManager, const std::string& folder, const std::string& name) {
	std::string texturePath = folder + name + ".png";

	TextureAsset* tex = TextureAsset::loadAsset(assetManager, texturePath, true);

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

	fontFaces[numFonts] = {
	        tex, charLocations, info[0].bounds.bottom - info[0].bounds.top,
	        digitWidth, maxCharWidth};

	AAsset_close(infoFile);

	return numFonts++;
}

#endif

void Text::deleteFonts() {
	for (int i = 0; i < numFonts; i++) {
		delete fontFaces[i].texture;
		delete[] fontFaces[i].charLocations;
	}
}

int Text::addText(float x, float y, std::string text, const Font& font, float size, const col& textColor) {
	FontFace fontFace = fontFaces[font.fontFaceId];
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
			charsToDraw[numChars++] = {{xPos, y}, fontFace.texture, bounds, textColor, scaleFactor};
			batchSize[batchToFill]++;
			xPos += (width + font.charSpacing) * scaleFactor;
		}
	}

	return length;
}

float Text::calculateWidth(std::string text, const Font& font, float size) {
	FontFace fontFace = fontFaces[font.fontFaceId];
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
		TextureAsset* tex = fontFaces[i].texture;

		int charNo = 0;
		for (int j = numCharsDrawn; j < nextBatchStart; j++) {
			CharToDraw& ctd = charsToDraw[j];
			if (ctd.texture == tex) {
				CharBounds bounds = ctd.bounds;
				vec2 pos = ctd.pos;

				float width = (bounds.right - bounds.left) * ctd.scaleFactor;
				float height = (bounds.bottom - bounds.top) * ctd.scaleFactor;

				vertices[charNo * 4 + 0] = {{pos.x, pos.y}, {bounds.left, bounds.top}, ctd.color};
				vertices[charNo * 4 + 1] = {{pos.x, pos.y - height}, {bounds.left, bounds.bottom}, ctd.color};
				vertices[charNo * 4 + 2] = {{pos.x + width, pos.y - height},
				                            {bounds.right, bounds.bottom},
				                            ctd.color};
				vertices[charNo * 4 + 3] = {{pos.x + width, pos.y}, {bounds.right, bounds.top}, ctd.color};

				indices[charNo * 6 + 0] = charNo * 4;
				indices[charNo * 6 + 1] = charNo * 4 + 1;
				indices[charNo * 6 + 2] = charNo * 4 + 2;
				indices[charNo * 6 + 3] = charNo * 4;
				indices[charNo * 6 + 4] = charNo * 4 + 2;
				indices[charNo * 6 + 5] = charNo * 4 + 3;

				charNo++;
			}
		}

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex->textureID);

		glBindVertexArray(vao);

		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(CharVertex) * charNo * 4, vertices, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Index) * charNo * 6, indices, GL_DYNAMIC_DRAW);

		// Draw as indexed triangles
		glDrawElements(GL_TRIANGLES, (GLsizei)charNo * 6, GL_UNSIGNED_SHORT, nullptr);
	}

	numCharsDrawn = nextBatchStart;
	batchSize[batch] = 0;
}

bool Text::initTextShader(
        const std::string& vertexSource,
        const std::string& fragmentSource,
        const std::string& positionAttributeName,
        const std::string& uvAttributeName,
        const std::string& colorAttributeName,
        const std::string& projectionMatrixUniformName) {
	GLuint vertexShader = 0;
	vertexShader = loadShader(GL_VERTEX_SHADER, vertexSource);
	if (!vertexShader) {
		return false;
	}

	GLuint fragmentShader = 0;
	fragmentShader = loadShader(GL_FRAGMENT_SHADER, fragmentSource);
	if (!fragmentShader) {
		glDeleteShader(vertexShader);
		return false;
	}

	GLuint iProgram = glCreateProgram();
	if (iProgram) {
		glAttachShader(iProgram, vertexShader);
		glAttachShader(iProgram, fragmentShader);

		glLinkProgram(iProgram);
		GLint linkStatus = GL_FALSE;
		glGetProgramiv(iProgram, GL_LINK_STATUS, &linkStatus);
		if (linkStatus != GL_TRUE) {
			GLint logLength = 0;
			glGetProgramiv(iProgram, GL_INFO_LOG_LENGTH, &logLength);

			// If we fail to link the shader program, log the result for debugging
			if (logLength) {
				GLchar* log = new GLchar[logLength];
				glGetProgramInfoLog(iProgram, logLength, nullptr, log);
				delete[] log;
			}

			glDeleteProgram(iProgram);
		} else {
			GLint positionAttribute = glGetAttribLocation(iProgram, positionAttributeName.c_str());
			GLint uvAttribute = glGetAttribLocation(iProgram, uvAttributeName.c_str());
			GLint colorAttribute = glGetAttribLocation(iProgram, colorAttributeName.c_str());
			GLint projectionMatrixUniform = glGetUniformLocation(iProgram,
			                                                     projectionMatrixUniformName.c_str());

			if (positionAttribute != -1 && uvAttribute != -1 && colorAttribute != -1 && projectionMatrixUniform != -1) {
				program = iProgram;
				position = positionAttribute;
				uv = uvAttribute;
				color = colorAttribute;
				projectionMatrix = projectionMatrixUniform;
			} else {
				glDeleteProgram(iProgram);
			}
		}
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	activateShader();

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

	glGenBuffers(1, &index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);

	// The position attribute is 3 floats
	glVertexAttribPointer(
	        position,          // attrib
	        2,                 // elements
	        GL_FLOAT,          // of type float
	        GL_FALSE,          // don't normalize
	        sizeof(CharVertex),// stride is Vertex bytes
	        0                  // pull from the start of the vertex data
	);
	glEnableVertexAttribArray(position);

	// The uv attribute is 2 floats
	glVertexAttribPointer(
	        uv,                   // attrib
	        2,                    // elements
	        GL_FLOAT,             // of type float
	        GL_FALSE,             // don't normalize
	        sizeof(CharVertex),   // stride is Vertex bytes
	        (uint8_t*)sizeof(vec2)// offset vec3 from the start
	);
	glEnableVertexAttribArray(uv);

	// The color attribute is 4 bytes
	glVertexAttribPointer(
	        color,                      // attrib
	        4,                          // elements
	        GL_UNSIGNED_BYTE,           // of type byte
	        GL_TRUE,                    // normalize
	        sizeof(CharVertex),         // stride is Vertex bytes
	        (uint8_t*)(sizeof(vec2) * 2)// offset vec3 + vec2 from the start
	);
	glEnableVertexAttribArray(color);

	return true;
}

void Text::updateProjectionMatrix() {
	mat4 projMat;
	buildOrthographicMatrix(&projMat, 1.0f, RATIO, -1.0f, 1.0f);
	glUniformMatrix4fv(projectionMatrix, 1, false, projMat.m16);
}
