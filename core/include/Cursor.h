#ifndef CURSOR_H
#define CURSOR_H

#include "Texture.h"
#include "Mesh.h"

struct CursorVertex {
	vec2 pos;
	vec2 uv;

	CursorVertex() = default;
	CursorVertex(vec2 pos, vec2 uv) : pos(pos), uv(uv) {}

	static void setupLayout();
};

struct Cursor {
	static Texture* tex;
	static vec2 pos;
	static float angle;
	static float size;
	static bool visible;

	static void init();

	static void drawCursor();

private:
	static std::unique_ptr<Mesh<CursorVertex>> mesh;
	static std::vector<CursorVertex> vertices;
	static std::vector<Index> indices;

	static void updateProjectionMatrix();
};

#endif// CURSOR_H
