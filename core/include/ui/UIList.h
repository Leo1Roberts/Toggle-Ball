#ifndef UI_VERTICAL_LIST_H
#define UI_VERTICAL_LIST_H

#include "UINode.h"


class UIList : public UINode {
public:
	UIList(bool vertical, float spacing, float scrollSpeed)
		: vertical(vertical), spacing(spacing), scrollSpeed(scrollSpeed) {}

	glm::vec2 measure() override;
	void arrangeChildren(Rectangle innerBounds) override;

	UIResponse processEvent(const Event& event) override;

	[[nodiscard]] float getOffset() const { return offset; }
	[[nodiscard]] float getTotalContentLength() const { return totalContentLength; }

private:
	void scrollTo(float offset);

	bool vertical;
	float spacing;
	float scrollSpeed;

	float offset = 0.f;
	float maxOffset{};
	float dragStartY{};
	float totalContentLength = 0.f;
};


class UIVerticalList : public UIList {
public:
	UIVerticalList(float spacing, float scrollSpeed = 20.f)
		: UIList(true, spacing, scrollSpeed) {}
};

class UIHorizontalList : public UIList {
public:
	UIHorizontalList(float spacing, float scrollSpeed = 20.f)
		: UIList(false, spacing, scrollSpeed) {}
};


#endif // UI_VERTICAL_LIST_H
