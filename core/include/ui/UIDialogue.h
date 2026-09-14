#ifndef UI_DIALOGUE_H
#define UI_DIALOGUE_H

#include "UIPanel.h"


class UIDialogue : public UIPanel {
public:
	UIDialogue();

	void setOnReturn(const std::function<void()>& callback) { onReturnCallback = dialogue->onReturnCallback = callback; }
	void setOnConfirm(const std::function<void()>& callback) { onConfirmCallback = dialogue->onConfirmCallback = callback; }

	UIResponse processEvent(const Event& event) override;

	void setLayout(Layout l) override;

	[[nodiscard]] bool isFocusable() const override { return true; }

protected:
	void addChildNode(std::unique_ptr<UINode> child) override {
		if (initialised)
			dialogue->addChild(std::move(child));
		else
			UINode::addChildNode(std::move(child));
	}

private:
	struct DialogueBox : UIPanel {
		explicit DialogueBox(const PanelStyle& style) : UIPanel(style) {}
		UIResponse processEvent(const Event& event) override;
		[[nodiscard]] bool isFocusable() const override { return true; }
		std::function<void()> onReturnCallback;
		std::function<void()> onConfirmCallback;
	};

	bool initialised = false;

	bool pressed = false;

	std::function<void()> onReturnCallback;
	std::function<void()> onConfirmCallback;

	DialogueBox* dialogue;
};


#endif // UI_DIALOGUE_H
