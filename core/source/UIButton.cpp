#include "UIButton.h"

#include "Settings.h"

UIButton::UIButton(const std::string& labelText, const ButtonStyle& bStyle)
	: UIPanel(bStyle.normalPanel), buttonStyle(bStyle) {
	if (!labelText.empty())
		labelNode = addChild(std::make_unique<UIText>(labelText, buttonStyle.normalText));
	updateVisualState();
}


UIResponse UIButton::processEvent(const Event& event) {
	if (state == ButtonState::Disabled)
		return UIResponse::Ignored;

	if (auto* key = std::get_if<KeyEvent>(&event)) {
		if (auto actionCode = Settings::Bindings->translate(key->chord)) {
			if (key->action == KeyAction::Down) {
				switch (*actionCode) {
				case ActionCode::Confirm:
					if (onClickCallback)
						onClickCallback();
					return UIResponse::RequestConfirm;
				case ActionCode::Cancel:
					return UIResponse::RequestCancel;
				default:;
				}
			}
		}
	}

	if (auto* pointer = std::get_if<PointerEvent>(&event)) {
		if (pointer->button == PointerButton::Primary) {
			switch (pointer->action) {
			case PointerAction::Down:
				pressed = true;
				updateVisualState();
				return UIResponse::Consumed;
			case PointerAction::Move:
				return UIResponse::Consumed;
			case PointerAction::Up:
				if (pressed) {
					pressed = false;
					updateVisualState();

					if (hovered && onClickCallback)
						onClickCallback();
				}
				return UIResponse::Consumed;
			default:
				break;
			}
		}
	}

	return UIResponse::Ignored;
}


void UIButton::onPointerEntered() {
	hovered = true;
	updateVisualState();
}

void UIButton::onPointerExited() {
	hovered = false;
	updateVisualState();
}


void UIButton::updateVisualState() {
	if (state == ButtonState::Disabled) return;

	if (pressed && hovered)
		state = ButtonState::Pressed;
	else if (hovered)
		state = ButtonState::Hovered;
	else
		state = ButtonState::Normal;
	
	switch (state) {
	case ButtonState::Normal:   style = buttonStyle.normalPanel;   break;
	case ButtonState::Hovered:  style = buttonStyle.hoveredPanel;  break;
	case ButtonState::Pressed:  style = buttonStyle.pressedPanel;  break;
	case ButtonState::Disabled: style = buttonStyle.disabledPanel; break;
	}

	if (labelNode) {
		switch (state) {
		case ButtonState::Normal:   labelNode->style = buttonStyle.normalText;   break;
		case ButtonState::Hovered:  labelNode->style = buttonStyle.hoveredText;  break;
		case ButtonState::Pressed:  labelNode->style = buttonStyle.pressedText;  break;
		case ButtonState::Disabled: labelNode->style = buttonStyle.disabledText; break;
		}
	}
}