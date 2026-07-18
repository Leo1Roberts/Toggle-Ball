#ifndef LAYOUT_H
#define LAYOUT_H
#include "VectorMatrix.h"


struct Rectangle {
	union {
		vec2 position{};
		struct {
			float x;
			float y;
		};
	};
	union {
		vec2 size{};
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

	vec2 offset{0.f};
};


#endif // LAYOUT_H