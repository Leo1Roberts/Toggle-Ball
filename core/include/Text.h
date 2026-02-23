#ifndef TEXT_H
#define TEXT_H

#include "Model.h"
#include "TextureAsset.h"

struct CharBounds {
	float left;
	float top;
	float right;
	float bottom;
};

struct FontFace {
	TextureAsset* texture;
	CharBounds* charLocations;
	float size;// Size (height) of font as a proportion of the texture height
	float digitWidth;
	float maxCharWidth;
};

struct Font {
	byte fontFaceId;
	bool monoSpaced;
	float charSpacing;
	float wordSpacing;
};

struct CharToDraw {
	vec2 pos;
	TextureAsset* texture;
	CharBounds bounds;
	col color;
	float scaleFactor;
};

struct CharVertex {
	vec2 pos;
	vec2 uv;
	col color;
};

class Text {
public:
#ifdef WINDOWS_VERSION
	static int loadFace(const std::string& folder, const std::string& name);// Returns index (font ID) of font face for fontFaces array, or -1 on fail
#else

	static byte loadFace(AAssetManager* assetManager, const std::string& folder,
	                     const std::string& name);// Returns index (font ID) of font face for fontFaces array, or -1 on fail
#endif

	static void deleteFonts();

	static void clearText() {// MUST be called each frame before adding text
		numChars = 0;
		numCharsDrawn = 0;
	}

	// Returns number of characters added (matches text.length() if successful)
	// [pos] is the position of the text in NDC
	// [size] is the height of the text in UI coordinates (height of screen is 2)
	static int addText(float x, float y, std::string text, const Font& font, float size, const col& textColor = BLACK);

	static float calculateWidth(std::string text, const Font& font, float size);

	static inline void markEndOfBatch() { batchToFill++; }

	static void drawText(byte batch);

	static bool initTextShader(
	        const std::string& vertexSource,
	        const std::string& fragmentSource,
	        const std::string& positionAttributeName,
	        const std::string& uvAttributeName,
	        const std::string& colorAttributeName,
	        const std::string& projectionMatrixUniformName);

	static void activateShader() { glUseProgram(program); };

	static void updateProjectionMatrix();

private:
	static FontFace fontFaces[];
	static int numFonts;

	static CharToDraw charsToDraw[];
	static int numChars;
	static int numCharsDrawn;
	static byte batchToFill;
	static unsigned short batchSize[];// If MAX_CHARS > 0x10000 this will need to be changed to an int

	static GLuint program;
	static GLint position;
	static GLint uv;
	static GLint color;
	static GLint projectionMatrix;

	static GLuint vao;
	static GLuint vertex_buffer;
	static GLuint index_buffer;
	static CharVertex vertices[];
	static Index indices[];
};

#endif// TEXT_H