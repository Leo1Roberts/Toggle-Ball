#include "ui/UIButton.h"

#include "Settings.h"
#include "ui/UIText.h"

UIButton::UIButton(const std::string& labelText, const ButtonStyle& bStyle)
	: UIPanel(bStyle.normalPanel), buttonStyle(bStyle) {
	if (!labelText.empty())
		labelNode = addChild(std::make_unique<UIText>(labelText, buttonStyle.normalText));
	updateStyle();
}


UIResponse UIButton::processEvent(const Event& event) {
	if (state == ButtonState::Disabled)
		return UIResponse::Ignored;

	if (auto* key = std::get_if<KeyEvent>(&event)) {
		if (key->action == KeyAction::Down) {
			switch (key->chord.code) {
			case KeyCode::Enter:
				if (onClickCallback)
					onClickCallback();
				return UIResponse::RequestConfirm;
			case KeyCode::Escape:
				return UIResponse::RequestCancel;
			default:;
			}
		}
	} else if (auto* pointer = std::get_if<PointerEvent>(&event)) {
		if (pointer->button == PointerButton::Primary) {
			switch (pointer->action) {
			case PointerAction::Down:
				pressed = true;
				updateStyle();
				return UIResponse::Consumed;
			case PointerAction::Up:
				if (pressed) {
					pressed = false;
					updateStyle();

					if (hovered && onClickCallback)
						onClickCallback();
				}
				return UIResponse::Consumed;
			default:;
			}
		}
	}

	return UIResponse::Ignored;
}


void UIButton::onPointerEntered() {
	hovered = true;
	updateStyle();
}
void UIButton::onPointerExited() {
	hovered = false;
	updateStyle();
}


void UIButton::updateStyle() {
	if (state != ButtonState::Disabled) {
		if (pressed && hovered)
			state = ButtonState::Pressed;
		else if (hovered)
			state = ButtonState::Hovered;
		else
			state = ButtonState::Normal;
	}
	
	switch (state) {
	case ButtonState::Normal:   panelStyle = buttonStyle.normalPanel;   break;
	case ButtonState::Hovered:  panelStyle = buttonStyle.hoveredPanel;  break;
	case ButtonState::Pressed:  panelStyle = buttonStyle.pressedPanel;  break;
	case ButtonState::Disabled: panelStyle = buttonStyle.disabledPanel; break;
	}

	if (labelNode) {
		switch (state) {
		case ButtonState::Normal:   labelNode->textStyle = buttonStyle.normalText;   break;
		case ButtonState::Hovered:  labelNode->textStyle = buttonStyle.hoveredText;  break;
		case ButtonState::Pressed:  labelNode->textStyle = buttonStyle.pressedText;  break;
		case ButtonState::Disabled: labelNode->textStyle = buttonStyle.disabledText; break;
		}
	}
}