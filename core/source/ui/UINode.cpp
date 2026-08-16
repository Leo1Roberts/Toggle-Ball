#include "ui/UINode.h"


glm::vec2 UINode::measure() {
	glm::vec2 contentSize{0.f, 0.f};

	if (layout.widthMode == SizingMode::Wrap || layout.heightMode == SizingMode::Wrap)
		for (auto& child : children) {
			if (!child->isActive()) continue;

			glm::vec2 childSize = child->measure() + child->layout.margin * 2.f + abs(child->layout.offset);
			contentSize = max(contentSize, childSize);
		}
	else
		for (auto& child : children)
			if (child->isActive())
				child->measure();

	switch (layout.widthMode) {
	case SizingMode::Absolute: measuredSize.x = layout.width; break;
	case SizingMode::Wrap:     measuredSize.x = contentSize.x + layout.padding.x * 2.f; break;
	case SizingMode::Stretch:  measuredSize.x = 0.f; break;
	}
	switch (layout.heightMode) {
	case SizingMode::Absolute: measuredSize.y = layout.height; break;
	case SizingMode::Wrap:     measuredSize.y = contentSize.y + layout.padding.y * 2.f; break;
	case SizingMode::Stretch:  measuredSize.y = 0.f; break;
	}

	return measuredSize;
}

void UINode::updateBounds(Rectangle parentBounds) {
	auto anchor = glm::vec2(0.f);

	switch (layout.anchor) {
		case Anchor::TopLeft:	   anchor = {0.0f, 0.0f}; break;
		case Anchor::TopCentre:	   anchor = {0.5f, 0.0f}; break;
		case Anchor::TopRight:	   anchor = {1.0f, 0.0f}; break;
		case Anchor::CentreLeft:   anchor = {0.0f, 0.5f}; break;
		case Anchor::Centre:	   anchor = {0.5f, 0.5f}; break;
		case Anchor::CentreRight:  anchor = {1.0f, 0.5f}; break;
		case Anchor::BottomLeft:   anchor = {0.0f, 1.0f}; break;
		case Anchor::BottomCentre: anchor = {0.5f, 1.0f}; break;
		case Anchor::BottomRight:  anchor = {1.0f, 1.0f}; break;
	}

	glm::vec2 availableSize = max(glm::vec2(0.f), parentBounds.size - layout.margin * 2.f);

	absoluteBounds.width()  = (layout.widthMode  == SizingMode::Stretch) ? availableSize.x * layout.width  : measuredSize.x;
	absoluteBounds.height() = (layout.heightMode == SizingMode::Stretch) ? availableSize.y * layout.height : measuredSize.y;

	absoluteBounds.position = parentBounds.position + layout.margin + (availableSize - absoluteBounds.size) * anchor + layout.offset;

	Rectangle innerBounds = absoluteBounds;
	innerBounds.position += layout.padding;
	innerBounds.size = max(glm::vec2(0.f), innerBounds.size - layout.padding * 2.f);

	arrangeChildren(innerBounds);
}

void UINode::arrangeChildren(Rectangle innerBounds) {
	for (auto& child: children)
		if (child->isActive())
			child->updateBounds(innerBounds);
}


bool UINode::contains(glm::vec2 point) const {
	if (point.x < absoluteBounds.x() || point.x > absoluteBounds.x() + absoluteBounds.width() ||
		point.y < absoluteBounds.y() || point.y > absoluteBounds.y() + absoluteBounds.height())
		return false;

	return containsPrecise(point);
}