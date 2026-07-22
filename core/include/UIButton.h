#ifndef UI_BUTTON_H
#define UI_BUTTON_H

#include "UIPanel.h"
#include "UIText.h"


enum class ButtonState {
	Normal,
	Hovered,
	Pressed,
	Disabled
};

class UIButton : public UIPanel {
public:
	explicit UIButton(const std::string& labelText = "", const ButtonStyle& bStyle = {});

	void setOnClick(const std::function<void()>& callback) { onClickCallback = callback; }

	UIResponse processEvent(const Event& event) override;

	void onPointerEntered() override;
	void onPointerExited() override;

	void disable() { state = ButtonState::Disabled; updateVisualState(); }
	void enable() { state = ButtonState::Normal; updateVisualState(); }

	void setStyle(const ButtonStyle& style) { buttonStyle = style; updateVisualState(); }

	[[nodiscard]] bool isFocusable() const override {
		return state != ButtonState::Disabled && isVisible() && isActive();
	}

private:
	bool hovered = false;
	bool pressed = false;
	ButtonState state = ButtonState::Normal;

	ButtonStyle buttonStyle;

	UIText* labelNode = nullptr;
	std::function<void()> onClickCallback;

	void updateVisualState();
};


#endif // UI_BUTTON_H
