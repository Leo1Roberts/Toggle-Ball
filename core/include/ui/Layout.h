#ifndef LAYOUT_H
#define LAYOUT_H

#include <glm/glm.hpp>


struct Rectangle {
	glm::vec2 position{0.f};
	glm::vec2 size{0.f};

	float& x() { return position.x; }
	[[nodiscard]] float x() const { return position.x; }
	float& y() { return position.y; }
	[[nodiscard]] float y() const { return position.y; }
	float& width() { return size.x; }
	[[nodiscard]] float width() const { return size.x; }
	float& height() { return size.y; }
	[[nodiscard]] float height() const { return size.y; }

	Rectangle operator*(float scale) const { return { position * scale, size * scale }; }
};


enum class Anchor {
	TopLeft,    TopCentre,    TopRight,
	CentreLeft, Centre,       CentreRight,
	BottomLeft, BottomCentre, BottomRight
};

enum class SizingMode { Absolute, Wrap, Stretch };

struct Layout {
	Anchor anchor = Anchor::TopLeft;

	SizingMode widthMode  = SizingMode::Stretch;
	float width  = 1.f;
	SizingMode heightMode = SizingMode::Stretch;
	float height = 1.f;

	glm::vec2 padding{0.f}; // Inside
	glm::vec2 margin{0.f}; // Outside
	glm::vec2 offset{0.f}; // Final shift
};


#endif // LAYOUT_H