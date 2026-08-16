#include "ui/UITextBox.h"


UITextBox::UITextBox(const TextInputBuffer::Validator& validator, const TextBoxStyle& bStyle, std::string placeholderText)
	: UIPanel(bStyle.normalPanel), textBoxStyle(bStyle), inputBuffer(validator), placeholder(std::move(placeholderText)) {
	container = addChild(std::make_unique<UINode>());
	container->setHitTestable(false);
	container->setHitTestableChildren(false); // Not safe to change - see updateCursorAndHighlight()
	container->setLayout({
		.anchor = Anchor::Centre,
		.widthMode  = SizingMode::Wrap,
		.heightMode = SizingMode::Wrap,
	});

	highlightContainer = container->addChild(std::make_unique<UINode>());
	textNode = container->addChild(std::make_unique<UIText>(placeholder, textBoxStyle.normalText));
	cursorNode = container->addChild(std::make_unique<UIPanel>(textBoxStyle.cursor));

	updateAppearance();
}


glm::vec2 UITextBox::measure() {
	bool cursorVisible = cursorNode ? cursorNode->isVisible() : false;
	bool highlightVisible = highlightContainer ? highlightContainer->isVisible() : false;

	cursorNode->hide();
	highlightContainer->hide();

	glm::vec2 size = UIPanel::measure();

	if (cursorVisible) cursorNode->show();
	if (highlightVisible) highlightContainer->show();

	return size;
}

void UITextBox::arrangeChildren(Rectangle innerBounds) {
	UIPanel::arrangeChildren(innerBounds);

	updateCursorAndHighlight();

	highlightContainer->measure();
	cursorNode->measure();

	Rectangle containerInner = container->getAbsoluteBounds();
	containerInner.position += container->getLayout().padding;
	containerInner.size = glm::max(glm::vec2(0.f), containerInner.size - container->getLayout().padding * 2.f);

	highlightContainer->updateBounds(containerInner);
	cursorNode->updateBounds(containerInner);
}


UIResponse UITextBox::processEvent(const Event& event) {
	if (state == TextBoxState::Disabled)
		return UIResponse::Ignored;

	auto effect = inputBuffer.processEvent(event);
	if (effect == TextInputEventEffect::Cursor ||
		effect == TextInputEventEffect::Buffer) {
		updateText();
		cursorInactiveTime = 0;
		if (onTextChangedCallback && effect == TextInputEventEffect::Buffer)
			onTextChangedCallback(*this);

		invalidateLayout();
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
	if (onFocusGainedCallback)
		onFocusGainedCallback();
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
	float cursorInactiveSeconds = toSeconds(cursorInactiveTime);
	if (focused && (cursorInactiveSeconds < 0.5f || (int)(cursorInactiveSeconds / 0.5f) % 2))
		cursorNode->show();
	else
		cursorNode->hide();

	if (!focused && valueProvider) {
		std::string newValue = valueProvider();
		if (newValue != textNode->getText())
			setText(newValue);
	}
}


int UITextBox::getPointedCursorIndex(glm::vec2 pointerPos) const {
	return textNode->getIndexAtPosition(pointerPos - textNode->getAbsoluteBounds().position, true);
}
int UITextBox::getPointedCharacterIndex(glm::vec2 pointerPos) const {
	return textNode->getIndexAtPosition(pointerPos - textNode->getAbsoluteBounds().position, false);
}

void UITextBox::updateText() {
	if (!focused && isEmpty())
		textNode->setText(placeholder);
	else
		textNode->setText(inputBuffer.getValue<const std::string&>());
}
void UITextBox::setText(const std::string& text) {
	inputBuffer.setText(text);
	updateText();
}

void UITextBox::updateCursorAndHighlight() {
	highlightContainer->clearChildren(); // Only safe because container children are not hit testable
	for (const auto& highlightRect : textNode->getHighlightRects(inputBuffer.getSelectionStartIndex(), inputBuffer.getSelectionEndIndex())) {
		auto highlightNode = highlightContainer->addChild(std::make_unique<UIPanel>(textBoxStyle.highlight));
		highlightNode->setLayout({
			.anchor = Anchor::TopLeft,
			.widthMode = SizingMode::Absolute, .width = highlightRect.width,
			.heightMode = SizingMode::Absolute, .height = highlightRect.height,
			.offset = highlightRect.position
		});
	}

	float cursorWidth = 1.f;
	glm::vec2 pos = textNode->getCursorPosition(inputBuffer.getCursorIndex());
	pos.x -= cursorWidth / 2.f;
	cursorNode->setLayout({
		.anchor = Anchor::TopLeft,
		.widthMode = SizingMode::Absolute, .width = cursorWidth,
		.heightMode = SizingMode::Absolute, .height = textNode->textStyle.fontSize,
		.offset = pos
	});
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
	}

	invalidateLayout();
}