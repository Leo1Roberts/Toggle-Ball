#include "ui/UIList.h"

void UIList::updateBounds(Rectangle parentBounds) {
	UINode::updateBounds(parentBounds);

	auto paddedBounds = getAbsoluteBounds();
	paddedBounds.position += padding;
	paddedBounds.size = max(glm::vec2(0.f), paddedBounds.size - padding * 2.f);
	float viewLength = vertical ? paddedBounds.height : paddedBounds.width;

	float freeLength = viewLength + spacing;
	float relativeSizeTotal = 0.f;
	for (const auto& child : getChildren()) {
		if (!child->isActive() || !child->isVisible())
			continue;

		freeLength -= spacing;
		if (vertical) {
			if (child->layout.heightMode == SizingMode::Absolute)
				freeLength -= child->layout.height;
			else if (child->layout.heightMode == SizingMode::Relative)
				relativeSizeTotal += child->layout.height;
		} else {
			if (child->layout.widthMode == SizingMode::Absolute)
				freeLength -= child->layout.width;
			else if (child->layout.widthMode == SizingMode::Relative)
				relativeSizeTotal += child->layout.width;
		}
	}

	freeLength = std::clamp(freeLength, 0.f, viewLength);
	float lengthPerRelativeSize = relativeSizeTotal > 0.f ? freeLength / relativeSizeTotal : 0.f;

	float currentPos = 0.f;
	for (const auto& child : getChildren()) {
		if (!child->isActive() || !child->isVisible())
			continue;

		child->layout.anchor = Anchor::TopLeft;

		Rectangle bounds;
		bounds = paddedBounds;
		if (vertical) {
			if (child->layout.widthMode == SizingMode::Absolute)
				child->layout.offset.y = currentPos - offset;
			else if (child->layout.heightMode == SizingMode::Relative) {
				bounds.y += currentPos - offset;
				bounds.height = lengthPerRelativeSize;
			}
		} else {
			if (child->layout.widthMode == SizingMode::Absolute)
				child->layout.offset.x = currentPos - offset;
			else if (child->layout.widthMode == SizingMode::Relative) {
				bounds.x += currentPos - offset;
				bounds.width = lengthPerRelativeSize;
			}
		}
		child->updateBounds(bounds);

		float childLength = vertical ? child->getAbsoluteBounds().height : child->getAbsoluteBounds().width;
		currentPos += childLength + spacing;
	}

	totalContentLength = currentPos > 0.f ? currentPos - spacing : 0.f;

	maxOffset = std::max(0.f, totalContentLength - viewLength);
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