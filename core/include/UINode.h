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

	template <typename T>
	T* addChild(std::unique_ptr<T> child) {
		child->parent = this;
		T* ptr = child.get();
		children.push_back(std::move(child));
		return ptr;
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

	void show() { visible = true; }
	void hide() { visible = false; }
	void activate() { active = true; }
	void deactivate() { active = false; }

	[[nodiscard]] const UINode* getParent() const { return parent; }
	[[nodiscard]] const std::vector<std::unique_ptr<UINode>>& getChildren() const { return children; }
	[[nodiscard]] bool isVisible() const { return visible; }
	[[nodiscard]] bool isActive() const { return active; }
	[[nodiscard]] bool isHitTestable() const { return hitTestable && active; }
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
