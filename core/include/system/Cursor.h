#ifndef CURSOR_H
#define CURSOR_H

#include "opengl/Mesh.h"
#include "opengl/Texture.h"

#include <optional>
#include <glm/glm.hpp>


struct CursorVertex {
	glm::vec2 pos;
	glm::vec2 uv;

	CursorVertex(glm::vec2 pos, glm::vec2 uv) : pos(pos), uv(uv) {}

	static void setupLayout();
};


struct Cursor {
	enum class Style : int { Invisible, Arrow, PointingHand, Text, HorizontalResize, VerticalResize, DynamicResize, COUNT };

	Style style = Style::Arrow;
	bool dynamic = false;
	bool captured = false;
	float angle = 0.f;

	bool operator==(const Cursor& other) const {
		return style == other.style && dynamic == other.dynamic &&
			(!dynamic || (captured == other.captured && angle == other.angle));
	}

	static void loadTextures();

	void drawDynamic(float size, glm::vec2 pos, glm::vec2 screenSize) const;

	[[nodiscard]] Texture* getTexture() const {
		return textures[(int)style].get();
	}

private:
	static std::unique_ptr<Texture> textures[];

	static std::vector<CursorVertex> vertices;
	static std::vector<Index> indices;
	static std::unique_ptr<Mesh<CursorVertex>> mesh;
};


struct ICursorProvider {
	virtual ~ICursorProvider() = default;

	virtual std::optional<Cursor> queryCursor() const { return std::nullopt; }
};


#endif // CURSOR_H
