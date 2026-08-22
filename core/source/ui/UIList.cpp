#include "ui/UIList.h"


glm::vec2 UIList::measure() {
	glm::vec2 contentSize{0.f, 0.f};
	int visibleCount = 0;

	for (auto& child : getChildren()) {
		if (!child->isActive()) continue;

		glm::vec2 childSize = child->measure();
		visibleCount++;

		const auto& childLayout = child->getLayout();
		float childOuterWidth  = childSize.x + (childLayout.margin.x * 2.f);
		float childOuterHeight = childSize.y + (childLayout.margin.y * 2.f);

		if (vertical) {
			if (childLayout.heightMode != SizingMode::Stretch)
				contentSize.y += childOuterHeight;
			contentSize.x = std::max(contentSize.x, childOuterWidth);
		} else {
			if (childLayout.widthMode != SizingMode::Stretch)
				contentSize.x += childOuterWidth;
			contentSize.y = std::max(contentSize.y, childOuterHeight);
		}
	}

	if (visibleCount > 1) {
		if (vertical)
			contentSize.y += spacing * (float)(visibleCount - 1);
		else
			contentSize.x += spacing * (float)(visibleCount - 1);
	}

	switch (layout.widthMode) {
	case SizingMode::Absolute:
		measuredSize.x = layout.width;
		break;
	case SizingMode::Stretch:
		if (!getParent() || getParent()->getLayout().widthMode != SizingMode::Wrap) {
			measuredSize.x = 0.f;
			break;
		}
	case SizingMode::Wrap:
		measuredSize.x = contentSize.x + layout.padding.x * 2.f;
		break;
	}
	switch (layout.heightMode) {
	case SizingMode::Absolute:
		measuredSize.y = layout.height;
		break;
	case SizingMode::Stretch:
		if (!getParent() || getParent()->getLayout().heightMode != SizingMode::Wrap) {
			measuredSize.y = 0.f;
			break;
		}
	case SizingMode::Wrap:
		measuredSize.y = contentSize.y + layout.padding.y * 2.f;
		break;
	}

	return measuredSize;
}

void UIList::arrangeChildren(Rectangle innerBounds) {
	float viewLength = vertical ? innerBounds.height() : innerBounds.width();
	float freeSpace = viewLength;
	float relativeWeightTotal = 0.f; // Use relative sizes as weights for distributing free space
	int visibleCount = 0;

	for (const auto& child : getChildren()) {
		if (!child->isActive()) continue;
		visibleCount++;

		const auto& childLayout = child->getLayout();

		if (vertical) {
			if (childLayout.heightMode == SizingMode::Stretch)
				relativeWeightTotal += childLayout.height;
			else
				freeSpace -= (child->getMeasuredSize().y + childLayout.margin.y * 2.f);
		} else {
			if (childLayout.widthMode == SizingMode::Stretch)
				relativeWeightTotal += childLayout.width;
			else
				freeSpace -= (child->getMeasuredSize().x + childLayout.margin.x * 2.f);
		}
	}

	if (visibleCount > 1)
		freeSpace -= spacing * (float)(visibleCount - 1);
	freeSpace = std::max(0.f, freeSpace);

	float spacePerWeight = (relativeWeightTotal > 0.f) ? (freeSpace / relativeWeightTotal) : 0.f;

	float currentPos = 0.f;

	for (const auto& child : getChildren()) {
		if (!child->isActive()) continue;

		const auto& childLayout = child->getLayout();
		auto slotBounds = innerBounds;

		if (vertical) {
			float childHeight = childLayout.heightMode == SizingMode::Stretch
			                    ? spacePerWeight * childLayout.height
			                    : child->getMeasuredSize().y;

			slotBounds.y() += currentPos - offset;

			slotBounds.height() = childLayout.heightMode == SizingMode::Stretch && childLayout.height > 0.f
			                    ? childHeight / childLayout.height + childLayout.margin.y * 2.f
			                    : childHeight + childLayout.margin.y * 2.f;

			currentPos += childHeight + childLayout.margin.y * 2.f + spacing;
		} else {
			float childWidth = childLayout.widthMode == SizingMode::Stretch
			                    ? spacePerWeight * childLayout.width
			                    : child->getMeasuredSize().x;

			slotBounds.x() += currentPos - offset;

			slotBounds.width() = childLayout.widthMode == SizingMode::Stretch && childLayout.width > 0.f
			                    ? childWidth / childLayout.width + childLayout.margin.x * 2.f
			                    : childWidth + childLayout.margin.x * 2.f;

			currentPos += childWidth + childLayout.margin.x * 2.f + spacing;
		}

		child->updateBounds(slotBounds);
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
	float newOffset = std::clamp(y, 0.f, maxOffset);
	if (newOffset != offset) {
		offset = newOffset;
		invalidateLayout();
	}
}