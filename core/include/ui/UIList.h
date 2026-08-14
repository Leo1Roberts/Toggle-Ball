#ifndef UI_VERTICAL_LIST_H
#define UI_VERTICAL_LIST_H

#include "UINode.h"


class UIList : public UINode {
public:
	UIList(bool vertical, glm::vec2 padding, float spacing, float scrollSpeed)
		: vertical(vertical), padding(padding), spacing(spacing), scrollSpeed(scrollSpeed) {}

	void updateBounds(Rectangle parentBounds) override;

	UIResponse processEvent(const Event& event) override;

	[[nodiscard]] float getOffset() const { return offset; }
	[[nodiscard]] float getTotalContentLength() const { return totalContentLength; }

private:
	void scrollTo(float offset);

	bool vertical;
	glm::vec2 padding;
	float spacing;
	float scrollSpeed;

	float offset = 0.f;
	float maxOffset{};
	float dragStartY{};
	float totalContentLength = 0.f;
};


class UIVerticalList : public UIList {
public:
	UIVerticalList(glm::vec2 padding, float spacing, float scrollSpeed = 20.f)
		: UIList(true, padding, spacing, scrollSpeed) {}
};

class UIHorizontalList : public UIList {
public:
	UIHorizontalList(glm::vec2 padding, float spacing, float scrollSpeed = 20.f)
		: UIList(false, padding, spacing, scrollSpeed) {}
};


#endif // UI_VERTICAL_LIST_H
