#ifndef UI_TEXT_BOX_H
#define UI_TEXT_BOX_H

#include "io/TextInputBuffer.h"
#include "ui/UIPanel.h"
#include "ui/UIText.h"


enum class TextBoxState {
	Normal,
	Hovered,
	Pressed,
	Focused,
	Disabled
};

class UITextBox : public UIPanel {
public:
	explicit UITextBox(const TextInputBuffer::Validator& validator, const TextBoxStyle& bStyle = {}, std::string placeholderText = "");

	void setOnFocusGained(const std::function<void()>& callback) { onFocusGainedCallback = callback; }
	void setOnCancel(const std::function<void(const UITextBox&)>& callback) { onCancelCallback = callback; }
	void setOnConfirm(const std::function<void(const UITextBox&)>& callback) { onConfirmCallback = callback; }
	void setOnTextChange(const std::function<void(const UITextBox&)>& callback) { onTextChangedCallback = callback; }
	void setValueProvider(const std::function<std::string()>& vp) { valueProvider = vp; }

	UIResponse processEvent(const Event& event) override;

	void onFocusGained() override;
	void onFocusLost(bool cancel) override;
	void onPointerEntered() override;
	void onPointerExited() override;

	void disable() { state = TextBoxState::Disabled; updateAppearance(); }
	void enable() { state = TextBoxState::Normal; updateAppearance(); }

	template <typename T>
	T getValue() const { return inputBuffer.getValue<T>(); }

	[[nodiscard]] bool isEmpty() const { return inputBuffer.isEmpty(); }
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
	std::string placeholder;

	std::function<void()> onFocusGainedCallback;
	std::function<void(const UITextBox&)> onCancelCallback;
	std::function<void(const UITextBox&)> onConfirmCallback;
	std::function<void(const UITextBox&)> onTextChangedCallback;
	std::function<std::string()> valueProvider;

	void updateText();
	void setText(const std::string& text);

	void updateCursorAndHighlight();
	void updateAppearance();

	microseconds cursorInactiveTime = 0;
};


#endif // UI_TEXT_BOX_H
