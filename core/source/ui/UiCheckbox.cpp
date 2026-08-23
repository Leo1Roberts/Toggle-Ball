#include "ui/UICheckbox.h"

UICheckbox::UICheckbox(const CheckboxStyle& style, State initialState)
	: state(initialState), checkboxStyle(style) {
	setOnTrigger([this] {
		state = state == State::Checked ? State::Unchecked : State::Checked;
		updateStyle();
		if (onCheckChangeCallback)
			onCheckChangeCallback(state == State::Checked);
	});
}


void UICheckbox::doUpdate(microseconds dt) {
	if (valueProvider) {
		state = valueProvider();
		updateStyle();
	}
}


void UICheckbox::updateStyle() {
	switch (state) {
	case State::Unchecked:
		panelStyle = checkboxStyle.unchecked;
		break;
	case State::Checked:
		panelStyle = checkboxStyle.checked;
		break;
	case State::Indeterminate:
		panelStyle = checkboxStyle.indeterminate;
		break;
	}
}