#ifndef UI_NODE_H
#define UI_NODE_H

#include "Event.h"
#include "Layout.h"

#include <memory>
#include <vector>


enum class UIResponse {
	Ignored,
	Consumed,
	RequestConfirm,
	RequestCancel
};


class UINode {
public:
	virtual ~UINode() = default;

	void addChild(std::unique_ptr<UINode> child) {
		child->parent = this;
		children.push_back(std::move(child));
	}

	virtual void updateLayout(Rectangle parentBounds);

	[[nodiscard]] bool contains(vec2 point) const;

	virtual void onFocusGained() {}
	virtual void onFocusLost(bool cancel) {}
	virtual void onPointerEntered() {}
	virtual void onPointerExited() {}

	virtual UIResponse processEvent(const Event& event) = 0;

	UINode* parent{nullptr};
	std::vector<std::unique_ptr<UINode>> children;

	bool isFocusable{false};

protected:
	[[nodiscard]] virtual bool containsPrecise(vec2 point) const { return true; }

private:
	Layout layout;
	Rectangle absoluteBounds;
	bool isVisible{true};
};


#endif // UI_NODE_H
