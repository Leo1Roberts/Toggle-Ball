#ifndef UI_TEXT_BUBBLE_H
#define UI_TEXT_BUBBLE_H

#include "UIPanel.h"

class UIText;


class UITextBubble : public UIPanel {
public:
	explicit UITextBubble(const std::string& text, glm::vec2 padding, const PanelStyle& pStyle, const TextStyle& tStyle = {});

	void updateBounds(Rectangle parentBounds) override;

	void setText(const std::string& text);

private:
	UIText* textNode = nullptr;

	glm::vec2 padding;
};


#endif // UI_TEXT_BUBBLE_H
