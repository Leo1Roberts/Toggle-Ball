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

class UIText;
class UIButton : public UIPanel {
public:
	explicit UIButton(const std::string& labelText = "", const ButtonStyle& bStyle = {});

	void setOnTrigger(const std::function<void()>& callback) { onTriggerCallback = callback; }

	UIResponse processEvent(const Event& event) override;

	void onPointerEntered() override;
	void onPointerExited() override;

	void disable() { state = ButtonState::Disabled; updateStyle(); }
	void enable() { state = ButtonState::Normal; updateStyle(); }

	void setText(const std::string& text) { labelNode->setText(text); }
	void setTextLayout(const Layout& l) { labelNode->setLayout(l); }

	void setButtonStyle(const ButtonStyle& style) { buttonStyle = style; updateStyle(); }

	[[nodiscard]] bool isFocusable() const override {
		return state != ButtonState::Disabled && isVisible() && isActive();
	}

protected:
	virtual void updateStyle();

private:
	bool hovered = false;
	bool pressed = false;
	ButtonState state = ButtonState::Normal;

	ButtonStyle buttonStyle;

	UIText* labelNode = nullptr;
	std::function<void()> onTriggerCallback;
};


#endif // UI_BUTTON_H
