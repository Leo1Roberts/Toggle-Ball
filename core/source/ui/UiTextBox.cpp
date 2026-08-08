#include "ui/UiTextBox.h"


UITextBox::UITextBox(const TextInputBuffer::Validator& validator, const TextBoxStyle& bStyle, std::string placeholderText)
	: UIPanel(bStyle.normalPanel), textBoxStyle(bStyle), inputBuffer(validator), placeholder(std::move(placeholderText)) {
	highlightContainer = addChild(std::make_unique<UINode>());
	highlightContainer->setHitTestable(false);

	textNode = addChild(std::make_unique<UIText>(placeholder, textBoxStyle.normalText));
	highlightContainer->layout = textNode->layout = {
		.anchor = Anchor::CentreLeft,
		.offset = {10.f, 0.f}
	};

	cursorNode = textNode->addChild(std::make_unique<UIPanel>(textBoxStyle.cursor));

	updateAppearance();
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
				inputBuffer.moveCursorTo(getPointedCursorIndex(pointer->position), pointer->modifiers & MOD_SHIFT);

				if (pointer->causedFocusChange || pointer->pointerDownCount == 3)
					inputBuffer.selectAll();
				else if (pointer->pointerDownCount == 2)
					inputBuffer.selectWord(getPointedCharacterIndex(pointer->position));

				cursorInactiveTime = 0;
				updateAppearance();
				return UIResponse::Consumed;
			case PointerAction::Up:
				if (pressed) {
					pressed = false;
					updateAppearance();
				}
				return UIResponse::Consumed;
			case PointerAction::StartDrag:
				return UIResponse::Consumed;
			default:;
			}
		}

		if (pressed && pointer->action == PointerAction::Drag) {
			inputBuffer.moveCursorTo(getPointedCursorIndex(pointer->position), true);
			cursorInactiveTime = 0;
			updateAppearance();
			return UIResponse::Consumed;
		}
	}

	return UIResponse::Ignored;
}


void UITextBox::onFocusGained() {
	focused = true;
	updateText();
	cursorInactiveTime = 0;
	inputBuffer.selectAll();
	updateAppearance();
}
void UITextBox::onFocusLost(bool cancel) {
	focused = false;
	updateText();
	inputBuffer.deselectAll();
	updateAppearance();
	if (cancel) {
		if (onCancelCallback)
			onCancelCallback(*this);
	} else {
		if (onConfirmCallback)
			onConfirmCallback(*this);
	}
}

void UITextBox::onPointerEntered() {
	hovered = true;
	updateAppearance();
}
void UITextBox::onPointerExited() {
	hovered = false;
	updateAppearance();
}


void UITextBox::doUpdate(microseconds dt) {
	cursorInactiveTime += dt;
	if (focused && (cursorInactiveTime / 500000 % 2 || cursorInactiveTime < 500000))
		cursorNode->show();
	else
		cursorNode->hide();

	if (!focused && valueProvider) {
		std::string newValue = valueProvider();
		if (newValue != textNode->getText()) {
			setText(newValue);
			updateBounds(getParent()->getAbsoluteBounds());
		}
	}
}


int UITextBox::getPointedCursorIndex(glm::vec2 pointerPos) const {
	return textNode->getIndexAtPosition(pointerPos - textNode->getAbsoluteBounds().position, true);
}
int UITextBox::getPointedCharacterIndex(glm::vec2 pointerPos) const {
	return textNode->getIndexAtPosition(pointerPos - textNode->getAbsoluteBounds().position, false);
}


void UITextBox::updateCursorAndHighlight() {
	highlightContainer->clearChildren();
	for (const auto& highlightRect : textNode->getHighlightRects(inputBuffer.getSelectionStartIndex(), inputBuffer.getSelectionEndIndex())) {
		auto highlightNode = highlightContainer->addChild(std::make_unique<UIPanel>(textBoxStyle.highlight));
		highlightNode->layout = {
			.anchor = Anchor::TopLeft,
			.widthMode = SizingMode::Absolute, .heightMode = SizingMode::Absolute,
			.width = highlightRect.width, .height = highlightRect.height,
			.offset = highlightRect.position
		};
		highlightNode->updateBounds(highlightContainer->getAbsoluteBounds());
	}

	float cursorWidth = 1.f;
	glm::vec2 pos = textNode->getCursorPosition(inputBuffer.getCursorIndex());
	pos.x -= cursorWidth / 2.f;
	cursorNode->layout = {
		.anchor = Anchor::TopLeft,
		.widthMode = SizingMode::Absolute, .heightMode = SizingMode::Absolute,
		.width = cursorWidth, .height = textNode->textStyle.fontSize,
		.offset = pos
	};

	cursorNode->updateBounds(textNode->getAbsoluteBounds());
}

void UITextBox::updateAppearance() {
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
		textNode->updateTextLayout();
		updateCursorAndHighlight();
	}
}