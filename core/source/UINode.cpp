#include "UINode.h"

void UINode::updateLayout(Rectangle parentBounds) {
	float ax = 0.0f;
	float ay = 0.0f;

	switch (layout.anchor) {
		case Anchor::TopLeft:      ax = 0.0f; ay = 0.0f; break;
		case Anchor::TopCentre:    ax = 0.5f; ay = 0.0f; break;
		case Anchor::TopRight:     ax = 1.0f; ay = 0.0f; break;
		case Anchor::CentreLeft:   ax = 0.0f; ay = 0.5f; break;
		case Anchor::Centre:       ax = 0.5f; ay = 0.5f; break;
		case Anchor::CentreRight:  ax = 1.0f; ay = 0.5f; break;
		case Anchor::BottomLeft:   ax = 0.0f; ay = 1.0f; break;
		case Anchor::BottomCentre: ax = 0.5f; ay = 1.0f; break;
		case Anchor::BottomRight:  ax = 1.0f; ay = 1.0f; break;
	}

	if (layout.widthMode == SizingMode::Absolute) {
		absoluteBounds.width = layout.width;
		absoluteBounds.x = parentBounds.x + (parentBounds.width - absoluteBounds.width) * ax + layout.offset.x;
	} else {
		absoluteBounds.width = (parentBounds.width * layout.width) - (2.0f * layout.offset.x);
		absoluteBounds.x = parentBounds.x + (parentBounds.width - absoluteBounds.width) * ax + layout.offset.x * (1.f - 2.f * ax);
	}

	if (layout.heightMode == SizingMode::Absolute) {
		absoluteBounds.height = layout.height;
		absoluteBounds.y = parentBounds.y + (parentBounds.height - absoluteBounds.height) * ay + layout.offset.y;
	} else {
		absoluteBounds.height = (parentBounds.height * layout.height) - (2.0f * layout.offset.y);
		absoluteBounds.y = parentBounds.y + (parentBounds.height - absoluteBounds.height) * ay + layout.offset.y * (1.f - 2.f * ay);
	}

	for (auto& child: children)
		child->updateLayout(absoluteBounds);
}

bool UINode::contains(vec2 point) const {
	if (point.x < absoluteBounds.x || point.x > absoluteBounds.x + absoluteBounds.width ||
		point.y < absoluteBounds.y || point.y > absoluteBounds.y + absoluteBounds.height)
		return false;

	return containsPrecise(point);
}