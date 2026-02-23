#ifndef CURSOR_H
#define CURSOR_H

#include "TextureAsset.h"

struct CursorVertex {
	vec2 pos;
	vec2 uv;
};

struct Cursor {
	static TextureAsset *arrowTex, *handTex, *resizeTex;

	static TextureAsset* tex;
	static vec2 pos;
	static float angle;
	static float size;
	static bool visible;

	static bool initShader(
	        const std::string& vertexSource,
	        const std::string& fragmentSource,
	        const std::string& positionAttributeName,
	        const std::string& uvAttributeName,
	        const std::string& projectionMatrixUniformName);

	static void activateShader() { glUseProgram(program); }

	static void updateProjectionMatrix();

	static void drawCursor();

private:
	static GLuint program;
	static GLint position;
	static GLint uv;
	static GLint projectionMatrix;

	static GLuint vao;
	static GLuint vertex_buffer;
	static CursorVertex vertices[6];
};

#endif// CURSOR_H
