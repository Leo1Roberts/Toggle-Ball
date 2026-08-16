#ifndef UI_NODE_H
#define UI_NODE_H

#include "io/Event.h"
#include "Layout.h"

#include <memory>
#include <vector>


enum class UIResponse {
	Ignored,
	Consumed,
	ConsumedNeedsHoverUpdate,
	RequestConfirm,
	RequestCancel
};


class UIManager;

class UINode {
public:
	UINode() = default;
	explicit UINode(bool hitTestable) : hitTestable(hitTestable) {}

	virtual ~UINode() = default;

	template <typename T>
	T* addChild(std::unique_ptr<T> child) {
		child->parent = this;
		T* ptr = child.get();
		children.push_back(std::move(child));
		invalidateLayout();
		return ptr;
	}
	template <typename T, typename... Args>
	T* addChild(Args&&... args) {
		return addChild<T>(std::make_unique<T>(std::forward<Args>(args)...));
	}

	void clearChildren() {
		children.clear();
		invalidateLayout();
	}

	virtual glm::vec2 measure();
	virtual void updateBounds(Rectangle parentBounds);
	virtual void arrangeChildren(Rectangle innerBounds);

	[[nodiscard]] bool contains(glm::vec2 point) const;

	virtual void onFocusGained() {}
	virtual void onFocusLost(bool cancel) {}
	virtual void onPointerEntered() {}
	virtual void onPointerExited() {}

	virtual UIResponse processEvent(const Event& event) { return UIResponse::Ignored; }

	void update(microseconds dt) {
		if (!active) return;
		doUpdate(dt);
		for (auto& child : children)
			child->update(dt);
	}

	void setAbsoluteBounds(Rectangle bounds) { absoluteBounds = bounds; }
	[[nodiscard]] Rectangle getAbsoluteBounds() const { return absoluteBounds; }

	virtual void submitRender(UIManager& manager) {}

	void invalidateLayout() {
		layoutInvalid = true;
		if (parent) parent->invalidateLayout();
	}
	void markLayoutValid() {
		layoutInvalid = false;
		for (auto& child : children)
			child->markLayoutValid();
	}

	void show() { visible = true; }
	void hide() { visible = false; }
	void activate() { active = true; }
	void deactivate() { active = false; }

	void setLayout(const Layout& l) { layout = l; invalidateLayout(); }
	void setHitTestable(bool canBeHit) { hitTestable = canBeHit; }
	void setHitTestableChildren(bool canBeHit) { hitTestableChildren = canBeHit; }

	[[nodiscard]] const Layout& getLayout() const { return layout; }
	[[nodiscard]] glm::vec2 getMeasuredSize() const { return measuredSize; }
	[[nodiscard]] bool layoutIsInvalid() const { return layoutInvalid; }
	[[nodiscard]] UINode* getParent() const { return parent; }
	[[nodiscard]] const std::vector<std::unique_ptr<UINode>>& getChildren() const { return children; }
	[[nodiscard]] bool isVisible() const { return visible; }
	[[nodiscard]] bool isActive() const { return active; }
	[[nodiscard]] bool isHitTestable() const { return hitTestable && active; }
	[[nodiscard]] bool childrenAreHitTestable() const { return hitTestableChildren && active; }
	[[nodiscard]] virtual bool isFocusable() const { return false; }

protected:
	[[nodiscard]] virtual bool containsPrecise(glm::vec2 point) const { return true; }

	Layout layout;
	glm::vec2 measuredSize{};
	Rectangle absoluteBounds;

private:
	virtual void doUpdate(microseconds dt) {}

	UINode* parent = nullptr;
	std::vector<std::unique_ptr<UINode>> children;
	bool infertile = false;

	bool layoutInvalid = true;
	bool visible = true;
	bool active = true;
	bool hitTestable = true;
	bool hitTestableChildren = true;
};


class IUIRenderer {
public:
	virtual ~IUIRenderer() = default;
	virtual void flush(const glm::mat4& projectionMatrix) = 0;
};


#endif // UI_NODE_H
