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


class UIManager;

class UINode {
public:
	virtual ~UINode() = default;

	void addChild(std::unique_ptr<UINode> child) {
		child->parent = this;
		children.push_back(std::move(child));
	}

	virtual void updateLayout(Rectangle parentBounds);

	[[nodiscard]] bool contains(glm::vec2 point) const;

	virtual void onFocusGained() {}
	virtual void onFocusLost(bool cancel) {}
	virtual void onPointerEntered() {}
	virtual void onPointerExited() {}

	virtual UIResponse processEvent(const Event& event) { return UIResponse::Ignored; }

	[[nodiscard]] Rectangle getAbsoluteBounds() const { return absoluteBounds; }

	virtual void submitRender(UIManager& manager) {}

	[[nodiscard]] const UINode* getParent() const { return parent; }
	[[nodiscard]] const std::vector<std::unique_ptr<UINode>>& getChildren() const { return children; }
	[[nodiscard]] bool isVisible() const { return visible; }
	[[nodiscard]] bool isActive() const { return active; }
	[[nodiscard]] bool isHitTestable() const { return hitTestable && active && visible; }
	[[nodiscard]] virtual bool isFocusable() const { return false; }

	Layout layout;

protected:
	[[nodiscard]] virtual bool containsPrecise(glm::vec2 point) const { return true; }

	bool hitTestable = true;

private:
	Rectangle absoluteBounds;

	UINode* parent = nullptr;
	std::vector<std::unique_ptr<UINode>> children;

	bool visible = true;
	bool active = true;
};


class IUIRenderer {
public:
	virtual ~IUIRenderer() = default;
	virtual void flush(const glm::mat4& projectionMatrix) = 0;
};


#endif // UI_NODE_H
