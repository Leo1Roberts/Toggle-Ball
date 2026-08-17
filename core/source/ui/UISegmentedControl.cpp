#include "ui/UISegmentedControl.h"

#include "ui/UIButton.h"
#include "ui/UIList.h"


UISegmentedControl::UISegmentedControl(const std::vector<std::string>& options, int defaultOption, const SegmentedControlStyle& style, float optionsSpacing)
	: UIPanel(style.track), selectedOption(defaultOption), segmentedControlStyle(style) {
	auto list = addChild<UIHorizontalList>(optionsSpacing, 0.f);
	list->setLayout({
		.anchor = Anchor::Centre,
		.widthMode = SizingMode::Wrap,
		.heightMode = SizingMode::Wrap,
	});
	for (int i = 0; i < options.size(); i++) {
		const auto& option = options[i];
		auto optionButton = list->addChild<UIButton>(option);
		optionButton->setOnTrigger([this, i] { selectOption(i); });
		optionButtons.push_back(optionButton);
	}
	updateStyle();
}


void UISegmentedControl::selectOption(int option) {
	if (option != selectedOption) {
		selectedOption = option;
		updateStyle();
		if (onSelectedOptionChangeCallback)
			onSelectedOptionChangeCallback(option);
	}
}


void UISegmentedControl::setOptionLayout(Layout l) {
	l.anchor = Anchor::Centre;
	for (const auto& optionButton : optionButtons)
		optionButton->setLayout(l);
}
void UISegmentedControl::setOptionTextLayout(const Layout& l) {
	for (const auto& optionButton : optionButtons)
		optionButton->setTextLayout(l);
}


void UISegmentedControl::doUpdate(microseconds dt) {
	if (valueProvider)
		selectOption(valueProvider());
}


void UISegmentedControl::updateStyle() {
	for (int i = 0; i < optionButtons.size(); i++) {
		auto optionButton = optionButtons[i];
		optionButton->setButtonStyle(
			i == selectedOption
			? segmentedControlStyle.selectedOption
			: segmentedControlStyle.option
		);
	}
}