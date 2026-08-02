#include "UiTextBox.h"


UITextBox::UITextBox(const TextInputBuffer::Validator& validator, const TextBoxStyle& bStyle)
	: UIPanel(bStyle.normalPanel), textBoxStyle(bStyle), inputBuffer(validator) {
	highlightNode = addChild(std::make_unique<UIPanel>(textBoxStyle.highlight));
	textNode = addChild(std::make_unique<UIText>("", textBoxStyle.normalText));
	textNode->layout = {
		.anchor = Anchor::CentreLeft,
		.offset = {10.f, 0.f}
	};
	cursorNode = addChild(std::make_unique<UIPanel>(textBoxStyle.cursor));

	updateCursorAndHighlight();
	updateStyle();
}


UIResponse UITextBox::processEvent(const Event& event) {
	if (state == TextBoxState::Disabled)
		return UIResponse::Ignored;

	if (inputBuffer.processEvent(event)) {
		updateText();
		updateCursorAndHighlight();
		if (onTextChangedCallback) {
			cursorInactiveTime = 0;
			onTextChangedCallback(*this);
		}
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
			case PointerAction::Down:
				pressed = true;
				inputBuffer.moveCursorTo(getPointedCursorIndex(pointer->position.x));
				cursorInactiveTime = 0;
				updateCursorAndHighlight();
				updateStyle();
				return UIResponse::Consumed;
			case PointerAction::Up:
				if (pressed) {
					pressed = false;
					updateStyle();
				}
				return UIResponse::Consumed;
			default:;
			}
		}

		if (pressed && pointer->action == PointerAction::Move) {
			inputBuffer.moveCursorTo(getPointedCursorIndex(pointer->position.x));
			cursorInactiveTime = 0;
			updateCursorAndHighlight();
			updateStyle();
			return UIResponse::Consumed;
		}
	}

	return UIResponse::Ignored;
}


void UITextBox::onFocusGained() {
	focused = true;
	cursorInactiveTime = 0;
	updateStyle();
}
void UITextBox::onFocusLost(bool cancel) {
	focused = false;
	updateStyle();
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
	updateStyle();
}
void UITextBox::onPointerExited() {
	hovered = false;
	updateStyle();
}


void UITextBox::doUpdate(microseconds dt) {
	cursorInactiveTime += dt;
	if (focused && (cursorInactiveTime / 500000 % 2 || cursorInactiveTime < 500000))
		cursorNode->show();
	else
		cursorNode->hide();
}


int UITextBox::getPointedCursorIndex(float pointerX) const {
	const auto& text = inputBuffer.getValue<const std::string&>();
	float cursorIndex = text.length();
	for (int i = 0; i < text.length(); i++)
		if (pointerX < textNode->getAbsoluteBounds().x + textNode->measure(0, i + 1).x - 0.5f * textNode->measure(i, i + 1).x) {
			cursorIndex = i;
			break;
		}
	return cursorIndex;
}


void UITextBox::updateCursorAndHighlight() {
	glm::vec2 start = textNode->layout.offset;;
	glm::vec2 offset = start;
	offset.x += textNode->measure(0, inputBuffer.getSelectionStartIndex()).x;

	highlightNode->layout = {
		.anchor = Anchor::CentreLeft,
		.widthMode = SizingMode::Absolute, .heightMode = SizingMode::Absolute,
		.width = textNode->measure(inputBuffer.getSelectionStartIndex(), inputBuffer.getSelectionEndIndex()).x,
		.height = textNode->textStyle.fontSize,
		.offset = offset
	};

	highlightNode->updateBounds(getAbsoluteBounds());

	offset = start;
	offset.x += textNode->measure(0, inputBuffer.getCursorIndex()).x;

	cursorNode->layout = {
		.anchor = Anchor::CentreLeft,
		.widthMode = SizingMode::Absolute, .heightMode = SizingMode::Absolute,
		.width = 1.f, .height = textNode->textStyle.fontSize,
		.offset = offset
	};

	cursorNode->updateBounds(getAbsoluteBounds());
}

void UITextBox::updateStyle() {
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