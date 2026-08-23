#ifndef UI_CHECKBOX_H
#define UI_CHECKBOX_H

#include "UIButton.h"


class UICheckbox : public UIButton {
public:
	enum class State { Unchecked, Checked, Indeterminate };

	explicit UICheckbox(const CheckboxStyle& style = {}, State initialState = State::Unchecked);

	void setOnCheckChange(const std::function<void(bool)>& callback) { onCheckChangeCallback = callback; }
	void setValueProvider(const std::function<State()>& provider) { valueProvider = provider; }

protected:
	void updateStyle() override;

private:
	void doUpdate(microseconds dt) override;

	State state;

	CheckboxStyle checkboxStyle;

	std::function<void(bool)> onCheckChangeCallback;
	std::function<State()> valueProvider;
};


#endif // UI_CHECKBOX_H
