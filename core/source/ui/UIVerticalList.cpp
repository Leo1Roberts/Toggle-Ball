#include "ui/UIVerticalList.h"

void UIVerticalList::updateBounds(Rectangle parentBounds) {
	UINode::updateBounds(parentBounds);

	float currentY = 0.f;

	auto paddedBounds = getAbsoluteBounds();
	paddedBounds.position += padding;
	paddedBounds.size = max(glm::vec2(0.f), paddedBounds.size - padding * 2.f);

	for (const auto& child : getChildren()) {
		if (!child->isActive() || !child->isVisible())
			continue;

		child->layout.anchor = Anchor::TopLeft;
		child->layout.offset.y = currentY - scrollY;

		child->updateBounds(paddedBounds);

		currentY += child->getAbsoluteBounds().height + spacing;
	}

	totalContentHeight = currentY > 0.f ? currentY - spacing : 0.f;

	maxScroll = std::max(0.f, totalContentHeight - (getAbsoluteBounds().height - padding.y * 2.f));
	scrollY = std::clamp(scrollY, 0.f, maxScroll);
}


UIResponse UIVerticalList::processEvent(const Event& event) {
	if (auto* pointer = std::get_if<PointerEvent>(&event)) {
		switch (pointer->action) {
		case PointerAction::Down:
		case PointerAction::Up:
			return UIResponse::Consumed;
		case PointerAction::Scroll:
			scrollTo(scrollY - pointer->scroll.y * scrollSpeed);
			return UIResponse::ConsumedNeedsHoverUpdate;
		case PointerAction::StartDrag:
			dragStartY = scrollY + pointer->position.y;
			return UIResponse::Consumed;
		case PointerAction::Drag:
			scrollTo(dragStartY - pointer->position.y);
			return UIResponse::ConsumedNeedsHoverUpdate;
		default:;
		}
	}

	return UIResponse::Ignored;
}


void UIVerticalList::scrollTo(float y) {
	scrollY = std::clamp(y, 0.f, maxScroll);
	updateBounds(getParent() ? getParent()->getAbsoluteBounds() : getAbsoluteBounds());
}