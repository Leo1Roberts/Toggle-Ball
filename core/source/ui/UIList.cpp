#include "ui/UIList.h"

void UIList::updateBounds(Rectangle parentBounds) {
	UINode::updateBounds(parentBounds);

	float currentPos = 0.f;

	auto paddedBounds = getAbsoluteBounds();
	paddedBounds.position += padding;
	paddedBounds.size = max(glm::vec2(0.f), paddedBounds.size - padding * 2.f);

	for (const auto& child : getChildren()) {
		if (!child->isActive() || !child->isVisible())
			continue;

		child->layout.anchor = Anchor::TopLeft;

		if (vertical)
			child->layout.offset.y = currentPos - offset;
		else
			child->layout.offset.x = currentPos - offset;

		child->updateBounds(paddedBounds);

		float childLength = vertical ? child->getAbsoluteBounds().height : child->getAbsoluteBounds().width;
		currentPos += childLength + spacing;
	}

	totalContentLength = currentPos > 0.f ? currentPos - spacing : 0.f;

	maxOffset = std::max(0.f, totalContentLength - (vertical ? paddedBounds.height : paddedBounds.width));
	offset = std::clamp(offset, 0.f, maxOffset);
}


UIResponse UIList::processEvent(const Event& event) {
	if (scrollSpeed > 0.f)
		if (auto* pointer = std::get_if<PointerEvent>(&event)) {
			switch (pointer->action) {
			case PointerAction::Down:
			case PointerAction::Up:
				return UIResponse::Consumed;
			case PointerAction::Scroll:
				scrollTo(offset - (vertical ? pointer->scroll.y : pointer->scroll.x) * scrollSpeed);
				return UIResponse::ConsumedNeedsHoverUpdate;
			case PointerAction::StartDrag:
				dragStartY = offset + (vertical ? pointer->position.y : pointer->position.x);
				return UIResponse::Consumed;
			case PointerAction::Drag:
				scrollTo(dragStartY - pointer->position.y);
				return UIResponse::ConsumedNeedsHoverUpdate;
			default:;
			}
		}

	return UIResponse::Ignored;
}


void UIList::scrollTo(float y) {
	offset = std::clamp(y, 0.f, maxOffset);
	updateBounds(getParent() ? getParent()->getAbsoluteBounds() : getAbsoluteBounds());
}