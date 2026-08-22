#include "ui/UiDropDownList.h"

#include "ui/UIButton.h"
#include "ui/UIList.h"


// Tracks if the main button was pressed this frame
class DropdownMainButton : public UIButton {
public:
	bool pressedThisFrame = false;

	using UIButton::UIButton;

	UIResponse processEvent(const Event& event) override {
		if (auto* ptr = std::get_if<PointerEvent>(&event))
			if (ptr->action == PointerAction::Down && ptr->button == PointerButton::Primary)
				pressedThisFrame = true;

		return UIButton::processEvent(event);
	}
};

// Tracks if the option is currently pressed, and guarantees cleanup on release
class DropdownOptionButton : public UIButton {
public:
	bool pointerDown = false;
	std::function<void()> onPointerUpCallback;

	using UIButton::UIButton;

	UIResponse processEvent(const Event& event) override {
		if (auto* ptr = std::get_if<PointerEvent>(&event))
			if (ptr->action == PointerAction::Down && ptr->button == PointerButton::Primary)
				pointerDown = true;

		auto response = UIButton::processEvent(event);

		if (auto* ptr = std::get_if<PointerEvent>(&event))
			if (ptr->action == PointerAction::Up && ptr->button == PointerButton::Primary) {
				pointerDown = false;
				if (onPointerUpCallback)
					onPointerUpCallback();
			}

		return response;
	}
};


UIDropDownList::UIDropDownList(const std::vector<std::string>& options, int defaultOption, const DropDownListStyle& style, float gapToList, float optionsSpacing, glm::vec2 optionsPadding)
	: UIVerticalList(gapToList, 0.f), selectedOption(defaultOption), keyboardHoveredOption(defaultOption), options(options), dropDownStyle(style) {
	std::string defaultText = (defaultOption >= 0 && defaultOption < options.size()) ? options[defaultOption] : "";

	mainButton = addChild<DropdownMainButton>(defaultText);
	mainButton->setButtonStyle(style.mainButton);

	mainButton->setOnTrigger([this] {
		if (ignoreNextMainButtonTrigger)
			ignoreNextMainButtonTrigger = false;
		else
			setOpen(!open);
	});

	optionsPanel = addChild<UIPanel>(style.listBackground);
	optionsPanel->setLayout({
		.anchor = Anchor::TopLeft,
		.widthMode  = SizingMode::Wrap,
		.heightMode = SizingMode::Wrap,
		.padding = optionsPadding
	});
	optionsPanel->setActive(open);

	auto optionsList = optionsPanel->addChild<UIVerticalList>(optionsSpacing, 0.f);
	optionsList->setLayout({
		.anchor = Anchor::Centre,
		.widthMode = SizingMode::Wrap,
		.heightMode = SizingMode::Wrap,
	});

	for (int i = 0; i < options.size(); i++) {
		const auto& option = options[i];

		auto optionButton = optionsList->addChild<DropdownOptionButton>(option);
		optionButton->setOnTrigger([this, i] { selectOption(i); });
		optionButton->onPointerUpCallback = [this] { setOpen(false); };

		optionButtons.push_back(optionButton);
	}

	updateStyle();
}

void UIDropDownList::selectOption(int option) {
	if (option == selectedOption) return;

	selectedOption = option;

	if (mainButton && option >= 0 && option < options.size())
		mainButton->setText(options[option]);

	updateStyle();

	if (onSelectedOptionChangeCallback)
		onSelectedOptionChangeCallback(option);
}

void UIDropDownList::setOpen(bool nowOpen) {
	if (nowOpen == open) return;

	open = nowOpen;
	optionsPanel->setActive(open);

	if (open) {
		keyboardHoveredOption = selectedOption;
		changeFocusCallback(this, false);
	}

	updateStyle();
}


void UIDropDownList::setLayout(Layout l) {
	auto margin = l.margin;

	l.margin = glm::vec2(0.f);
	mainButton->setLayout(l);

	l.margin = margin;
	l.padding = glm::vec2(0.f);
	l.widthMode = l.heightMode = SizingMode::Wrap;
	UIVerticalList::setLayout(l);
}
void UIDropDownList::setOptionLayout(const Layout& l) {
	for (const auto& optionButton : optionButtons)
		optionButton->setLayout(l);
}
void UIDropDownList::setOptionTextLayout(const Layout& l) {
	for (const auto& optionButton : optionButtons)
		optionButton->setTextLayout(l);
}


UIResponse UIDropDownList::processEvent(const Event& event) {
	if (open) {
		if (auto* key = std::get_if<KeyEvent>(&event)) {
			if (key->action == KeyAction::Down || key->action == KeyAction::Repeat) {
				switch (key->chord.code) {
				case KeyCode::Up:
					keyboardHoveredOption = std::max(0, keyboardHoveredOption - 1);
					updateStyle();
					return UIResponse::Consumed;
				case KeyCode::Down:
					keyboardHoveredOption = std::min((int)options.size() - 1, keyboardHoveredOption + 1);
					updateStyle();
					return UIResponse::Consumed;
				case KeyCode::Enter:
					selectOption(keyboardHoveredOption);
					setOpen(false);
					return UIResponse::RequestConfirm;
				case KeyCode::Escape:
					setOpen(false);
					return UIResponse::RequestCancel;
				default:
					break;
				}
			}
		}
	}

	return UIVerticalList::processEvent(event);
}


void UIDropDownList::doUpdate(microseconds dt) {
	if (pendingFocusLossClose) {
		pendingFocusLossClose = false;

		bool mainButtonPressed = mainButton->pressedThisFrame;

		bool optionPressed = false;
		for (auto* btn : optionButtons) {
			if (btn->pointerDown) {
				optionPressed = true;
				break;
			}
		}

		if (mainButtonPressed) {
			ignoreNextMainButtonTrigger = true;
			setOpen(false);
		}
		else if (!optionPressed)
			setOpen(false);
	}

	mainButton->pressedThisFrame = false;

	if (valueProvider)
		selectOption(valueProvider());
}

void UIDropDownList::updateStyle() {
	for (int i = 0; i < optionButtons.size(); i++) {
		auto optionButton = optionButtons[i];
		optionButton->setButtonStyle(
			(open ? i == keyboardHoveredOption : i == selectedOption)
			? dropDownStyle.selectedOption
			: dropDownStyle.option
		);
	}
}