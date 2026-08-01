#include "UiTextBox.h"

#include "UIText.h"


UITextBox::UITextBox(const TextInputBuffer::Validator& validator, const TextBoxStyle& bStyle)
	: UIPanel(bStyle.normalPanel), textBoxStyle(bStyle), inputBuffer(validator) {
	textNode = addChild(std::make_unique<UIText>("", textBoxStyle.normalText));
	textNode->layout = {
		.anchor = Anchor::CentreLeft,
		.offset = {10.f, 0.f}
	};
	updateVisualState();
}


UIResponse UITextBox::processEvent(const Event& event) {
	if (state == TextBoxState::Disabled)
		return UIResponse::Ignored;

	if (inputBuffer.processEvent(event)) {
		updateText();
		if (onTextChangedCallback)
			onTextChangedCallback(*this);
		return UIResponse::Consumed;
	}

	if (auto* key = std::get_if<KeyEvent>(&event)) {
		if (key->action == KeyAction::Down) {
			switch (key->chord.code) {
			case KeyCode::Enter:
				return UIResponse::RequestConfirm;
			case KeyCode::Escape:
				return UIResponse::RequestCancel;
			default:;
			}
		}
	} else if (auto* pointer = std::get_if<PointerEvent>(&event)) {
		if (pointer->button == PointerButton::Primary) {
			switch (pointer->action) {
			case PointerAction::Down: // TODO: move text cursor
				pressed = true;
				updateVisualState();
				return UIResponse::Consumed;
			case PointerAction::Move:
				return UIResponse::Consumed;
			case PointerAction::Up:
				if (pressed) {
					pressed = false;
					updateVisualState();
				}
				return UIResponse::Consumed;
			default:;
			}
		}
	}

	return UIResponse::Ignored;
}


void UITextBox::onFocusGained() {
	focused = true;
	updateVisualState();
}
void UITextBox::onFocusLost(bool cancel) {
	focused = false;
	updateVisualState();
	if (cancel) {
		if (onCancelCallback)
			onCancelCallback();
	} else {
		if (onConfirmCallback)
			onConfirmCallback(*this);
	}
	updateText();
}

void UITextBox::onPointerEntered() {
	hovered = true;
	updateVisualState();
}
void UITextBox::onPointerExited() {
	hovered = false;
	updateVisualState();
}


void UITextBox::updateText() {
	textNode->text = inputBuffer.getValue<std::string>();
}

void UITextBox::updateVisualState() {
	if (state != TextBoxState::Disabled) {
		if (focused)
			state = TextBoxState::Focused;
		else if (pressed && hovered)
			state = TextBoxState::Pressed;
		else if (hovered)
			state = TextBoxState::Hovered;
		else
			state = TextBoxState::Normal;
	}
	
	switch (state) {
	case TextBoxState::Normal:   panelStyle = textBoxStyle.normalPanel;   break;
	case TextBoxState::Hovered:  panelStyle = textBoxStyle.hoveredPanel;  break;
	case TextBoxState::Pressed:
	case TextBoxState::Focused:  panelStyle = textBoxStyle.focusedPanel;  break;
	case TextBoxState::Disabled: panelStyle = textBoxStyle.disabledPanel; break;
	}

	if (textNode) {
		switch (state) {
		case TextBoxState::Normal:   textNode->textStyle = textBoxStyle.normalText;   break;
		case TextBoxState::Hovered:  textNode->textStyle = textBoxStyle.hoveredText;  break;
		case TextBoxState::Pressed:
		case TextBoxState::Focused:  textNode->textStyle = textBoxStyle.focusedText;  break;
		case TextBoxState::Disabled: textNode->textStyle = textBoxStyle.disabledText; break;
		}
	}
}