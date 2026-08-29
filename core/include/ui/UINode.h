#ifndef UI_NODE_H
#define UI_NODE_H

#include "io/Event.h"
#include "Layout.h"
#include "system/Cursor.h"

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

class UINode : public ICursorProvider {
public:
	~UINode() override = default;

	template <typename T>
	T* addChild(std::unique_ptr<T> child) {
		child->parent = this;
		child->setChangeFocusCallback(changeFocusCallback);
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

	void show() { visible = true; invalidateLayout(); }
	void hide() { visible = false; invalidateLayout(); }
	void activate() { active = true; invalidateLayout(); }
	void deactivate() { active = false; invalidateLayout(); }
	void setActive(bool nowActive) { active = nowActive; invalidateLayout(); }

	virtual void setLayout(Layout l) { layout = l; invalidateLayout(); }
	void setHitTestable(bool canBeHit) { hitTestable = canBeHit; }
	void setHitTestableChildren(bool canBeHit) { hitTestableChildren = canBeHit; }

	void setChangeFocusCallback(const std::function<bool(UINode*, bool)>& callback) {
		changeFocusCallback = callback;
	}

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

	[[nodiscard]] std::optional<Cursor> queryCursor() const override { return std::nullopt; }

	bool isOverlay = false;

protected:
	UINode() = default;

	[[nodiscard]] virtual bool containsPrecise(glm::vec2 point) const { return true; }

	Layout layout;
	glm::vec2 measuredSize{};
	Rectangle absoluteBounds;

	std::function<bool(UINode*, bool)> changeFocusCallback;

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
