#ifndef UI_VERTICAL_LIST_H
#define UI_VERTICAL_LIST_H

#include "UINode.h"


class UIVerticalList : public UINode {
public:
	UIVerticalList(glm::vec2 padding, float spacing, float scrollSpeed = 20.f)
		: padding(padding), spacing(spacing), scrollSpeed(scrollSpeed) {}

	void updateBounds(Rectangle parentBounds) override;

	UIResponse processEvent(const Event& event) override;

	[[nodiscard]] float getScrollY() const { return scrollY; }
	[[nodiscard]] float getTotalContentHeight() const { return totalContentHeight; }

private:
	void scrollTo(float y);

	glm::vec2 padding;
	float spacing;
	float scrollSpeed;
	float scrollY = 0.f;
	float totalContentHeight = 0.f;
	float maxScroll{};
	float dragStartY{};
};


#endif // UI_VERTICAL_LIST_H
