#ifndef UI_TOGGLE_H
#define UI_TOGGLE_H

#include "UIButton.h"


class UIToggle : public UIPanel {
public:
	explicit UIToggle(bool initialState, const ToggleStyle& style = {});

	void setOnToggle(const std::function<void(bool, byte)>& callback) { onToggleCallback = callback; }
	void setValueProvider(const std::function<float()>& provider) { valueProvider = provider; }

	UIResponse processEvent(const Event& event) override;

	void onPointerEntered() override;
	void onPointerExited() override;

	[[nodiscard]] bool isFocusable() const override { return isVisible() && isActive(); }

	void setHandleLayout(const Layout& layout) { handleNode->setLayout(layout); }
	[[nodiscard]] const Layout& getHandleLayout() const { return handleNode->getLayout(); }

private:
	void doUpdate(microseconds dt) override;

	[[nodiscard]] float getTrackWidth() const;
	void updateVisuals();

	ToggleStyle toggleStyle;
	UIPanel* handleNode = nullptr;

	bool state;
	float handlePosition;

	bool hovered = false;
	bool pressed = false;
	ButtonState visualState = ButtonState::Normal;

	bool dragging = false;
	float dragStartX{};
	float dragStartHandlePosition{};

	std::function<void(bool, byte)> onToggleCallback;
	std::function<float()> valueProvider; // If set, this will be used to set the handle position
};


#endif // UI_TOGGLE_H
