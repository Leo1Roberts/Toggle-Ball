#ifndef LAYOUT_H
#define LAYOUT_H

#include <glm/glm.hpp>


struct Rectangle {
	union {
		glm::vec2 position{};
		struct {
			float x;
			float y;
		};
	};
	union {
		glm::vec2 size{};
		struct {
			float width;
			float height;
		};
	};
};


enum class Anchor {
	TopLeft,    TopCentre,    TopRight,
	CentreLeft, Centre,       CentreRight,
	BottomLeft, BottomCentre, BottomRight
};

enum class SizingMode {
	Absolute,
	Relative
};

struct Layout {
	Anchor anchor = Anchor::TopLeft;

	SizingMode widthMode  = SizingMode::Relative;
	SizingMode heightMode = SizingMode::Relative;

	float width  = 1.f;
	float height = 1.f;

	glm::vec2 offset{0.f};
};


#endif // LAYOUT_H