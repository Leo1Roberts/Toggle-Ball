#ifndef UI_TEXT_BOX_H
#define UI_TEXT_BOX_H

#include "TextInputBuffer.h"
#include "UIPanel.h"
#include "UIText.h"


enum class TextBoxState {
	Normal,
	Hovered,
	Pressed,
	Focused,
	Disabled
};

class UITextBox : public UIPanel {
public:
	explicit UITextBox(const TextInputBuffer::Validator& validator, const TextBoxStyle& bStyle = {});

	void setOnCancel(const std::function<void()>& callback) { onCancelCallback = callback; }
	void setOnConfirm(const std::function<void(const UITextBox&)>& callback) { onConfirmCallback = callback; }
	void setOnTextChange(const std::function<void(const UITextBox&)>& callback) { onTextChangedCallback = callback; }

	UIResponse processEvent(const Event& event) override;

	void onFocusGained() override;
	void onFocusLost(bool cancel) override;
	void onPointerEntered() override;
	void onPointerExited() override;

	void disable() { state = TextBoxState::Disabled; updateStyle(); }
	void enable() { state = TextBoxState::Normal; updateStyle(); }

	template <typename T>
	T getValue() const { return inputBuffer.getValue<T>(); }

	[[nodiscard]] bool isFocusable() const override { return isVisible() && isActive(); }

private:
	void doUpdate(microseconds dt) override;

	[[nodiscard]] int getPointedCursorIndex(glm::vec2 pointerPos) const;
	[[nodiscard]] int getPointedCharacterIndex(glm::vec2 pointerPos) const;

	bool focused = false;
	bool hovered = false;
	bool pressed = false;
	TextBoxState state = TextBoxState::Normal;

	TextBoxStyle textBoxStyle;

	UIText* textNode = nullptr;
	UIPanel* cursorNode = nullptr;
	UINode* highlightContainer = nullptr;
	TextInputBuffer inputBuffer;

	std::function<void()> onCancelCallback;
	std::function<void(const UITextBox&)> onConfirmCallback;
	std::function<void(const UITextBox&)> onTextChangedCallback;

	void updateText() { textNode->setText(inputBuffer.getValue<const std::string&>()); }
	void updateCursorAndHighlight();
	void updateStyle();

	microseconds cursorInactiveTime = 0;
};


#endif // UI_TEXT_BOX_H
