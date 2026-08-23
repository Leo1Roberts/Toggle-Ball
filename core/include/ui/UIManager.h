#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include "Settings.h"
#include "UIContainer.h"
#include "UIPanel.h"
#include "UIText.h"

#include <glm/glm.hpp>
#include <unordered_map>


class UIManager {
public:
	UIManager() {
		rootNode.setChangeFocusCallback([this](UINode* newFocus, bool cancel) {
			return changeFocus(newFocus, cancel);
		});
	}

	void resize(int screenWidth, int screenHeight, float screenDPIScale);

	bool changeFocus(UINode* newFocus, bool cancel);

	bool processEvent(const Event& event);

	void update(microseconds dt);

	void submitPanel(const UIPanel* panel);
	void submitText(const UIText* text);
	void render();

	template <typename T>
	T* addNode(std::unique_ptr<T> node) { return rootNode.addChild(std::move(node)); }
	template <typename T, typename... Args>
	T* addNode(Args&&... args) { return rootNode.addChild<T>(std::forward<Args>(args)...); }

	void removeAllChildrenOfNode(UINode* node);

	[[nodiscard]] float getScale() const { return dpiScale * Settings::Sizes.uiScale; }
	[[nodiscard]] glm::vec2 getLogicalScreenSize() const { return logicalScreenSize; }
	[[nodiscard]] const glm::mat4& getProjectionMatrix() const { return projectionMatrix; }

private:
	[[nodiscard]] UINode* findNodePointedTo(glm::vec2 pointerPosition);
	[[nodiscard]] static UINode* findNodePointedToRecursive(UINode* currentNode, glm::vec2 pointerPosition);

	std::vector<UINode*> overlays;
	void drawNodeRecursive(UINode* node, bool drawingOverlays);

	[[nodiscard]] glm::vec2 screenToLogicalPosition(glm::vec2 screenPosition) const { return screenPosition / getScale(); }

	void setRenderer(IUIRenderer* newRenderer);

	void unregisterNode(UINode* node);

	UIContainer rootNode;
	UINode* focusedNode = nullptr;
	std::unordered_map<int, UINode*> hoveredNodes;
	std::unordered_map<int, UINode*> downCapturedNodes;
	std::unordered_map<int, UINode*> dragCapturedNodes;

	float dpiScale = 1.f;
	glm::vec2 logicalScreenSize{};
	glm::mat4 projectionMatrix{};

	IUIRenderer* activeRenderer = nullptr;
	UIPanelRenderer panelRenderer;
	UITextRenderer textRenderer;
};


#endif // UI_MANAGER_H