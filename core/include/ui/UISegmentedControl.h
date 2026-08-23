#ifndef UI_SEGMENTED_CONTROL_H
#define UI_SEGMENTED_CONTROL_H

#include "UIPanel.h"

class UIButton;


class UISegmentedControl : public UIPanel {
public:
	UISegmentedControl(const std::vector<std::string>& options, int defaultOption, const SegmentedControlStyle& style = {}, float optionsSpacing = 0.f);

	void setOnSelectedOptionChange(const std::function<void(int)>& callback) { onSelectedOptionChangeCallback = callback; }
	void setValueProvider(const std::function<int()>& provider) { valueProvider = provider; }

	void selectOption(int option);

	void setOptionLayout(Layout l);
	void setOptionTextLayout(const Layout& l);

private:
	void doUpdate(microseconds dt) override;

	int selectedOption;

	SegmentedControlStyle segmentedControlStyle;

	std::vector<UIButton*> optionButtons;
	std::function<void(int)> onSelectedOptionChangeCallback;
	std::function<int()> valueProvider;

	void updateStyle();
};


#endif // UI_SEGMENTED_CONTROL_H
