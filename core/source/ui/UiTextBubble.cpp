#include "ui/UiTextBubble.h"

#include "ui/UIText.h"


UITextBubble::UITextBubble(const std::string& text, glm::vec2 padding, const PanelStyle& pStyle, const TextStyle& tStyle)
	: UIPanel(pStyle), padding(padding) {
	textNode = addChild(std::make_unique<UIText>(text, tStyle));
}


void UITextBubble::updateBounds(Rectangle parentBounds) { // Similar to default implementation
	float ax = 0.f;
	float ay = 0.f;

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

	absoluteBounds.width = textNode->getTextLayout().totalSize.x + padding.x * 2.f;
	absoluteBounds.x = parentBounds.x + (parentBounds.width - absoluteBounds.width) * ax + layout.offset.x;

	absoluteBounds.height = textNode->getTextLayout().totalSize.y + padding.y * 2.f;
	absoluteBounds.y = parentBounds.y + (parentBounds.height - absoluteBounds.height) * ay + layout.offset.y;

	for (auto& child: getChildren())
		child->updateBounds(absoluteBounds);
}


void UITextBubble::setText(const std::string& text) {
	textNode->setText(text);
	updateBounds(getParent()->getAbsoluteBounds());
}