#ifndef BOX_H
#define BOX_H

#include <glm/glm.hpp>

// For use with larger y = higher up
struct SelectBox {
	SelectBox() = default;

	explicit SelectBox(glm::vec2 point)
		: left(point.x), right(point.x), bottom(point.y), top(point.y) {}

	SelectBox(float x1, float x2, float y1, float y2) {
		if (x1 < x2) {
			left = x1; right = x2;
		} else {
			left = x2; right = x1;
		}
		if (y1 < y2) {
			bottom = y1; top = y2;
		} else {
			bottom = y2; top = y1;
		}
	}

	[[nodiscard]] bool touchesCircle(glm::vec2 centre, float radius) const;

	float left, right, bottom, top;
};


#endif // BOX_H
