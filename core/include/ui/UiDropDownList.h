#ifndef UI_DROP_DOWN_LIST_H
#define UI_DROP_DOWN_LIST_H

#include "UIList.h"
#include "UIStyle.h"

class UIPanel;
class UIButton;


class UIDropDownList : public UIVerticalList {
public:
	UIDropDownList(const std::vector<std::string>& options, int defaultOption, const DropDownListStyle& style = {}, float gapToList = 0.f, float optionsSpacing = 0.f, glm::vec2 optionsPadding = glm::vec2(0.f));

	void setOnSelectedOptionChange(const std::function<void(int)>& callback) { onSelectedOptionChangeCallback = callback; }
	void setValueProvider(const std::function<int()>& provider) { valueProvider = provider; }

	void selectOption(int option);
	void setOpen(bool nowOpen);

	void setLayout(Layout l) override;
	void setOptionLayout(const Layout& l);
	void setOptionTextLayout(const Layout& l);

	[[nodiscard]] bool isFocusable() const override { return isVisible() && isActive(); }
	void onFocusLost(bool cancel) override { if (open) pendingFocusLossClose = true; }

	UIResponse processEvent(const Event& event) override;

private:
	void doUpdate(microseconds dt) override;
	void updateStyle();

	bool open = false;
	int selectedOption;
	int keyboardHoveredOption;
	bool pendingFocusLossClose = false;
	bool ignoreNextMainButtonTrigger = false;
	std::vector<std::string> options;

	DropDownListStyle dropDownStyle;

	class DropdownMainButton* mainButton;
	UIPanel* optionsPanel;
	std::vector<class DropdownOptionButton*> optionButtons;

	std::function<void(int)> onSelectedOptionChangeCallback;
	std::function<int()> valueProvider; // If set, this will be used to set the selected option
};


#endif // UI_DROP_DOWN_LIST_H
