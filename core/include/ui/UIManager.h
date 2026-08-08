#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include "Settings.h"
#include "UIPanel.h"
#include "UIText.h"

#include <glm/glm.hpp>
#include <unordered_map>


class UIManager {
public:
	void resize(int screenWidth, int screenHeight, float screenDPIScale);

	bool changeFocus(UINode* newFocus, bool cancel);

	bool processEvent(const Event& event);

	void update(microseconds dt) { rootNode.update(dt); }

	void submitPanel(const UIPanel* panel);
	void submitText(const UIText* text);
	void render();

	template <typename T>
	T* addNode(std::unique_ptr<T> node) { return rootNode.addChild(std::move(node)); }

	void removeAllChildrenOfNode(UINode* node);

	[[nodiscard]] float getScale() const { return dpiScale * Settings::Sizes.uiScale; }
	[[nodiscard]] const glm::mat4& getProjectionMatrix() const { return projectionMatrix; }

private:
	[[nodiscard]] static UINode* findNodePointedTo(UINode* currentNode, glm::vec2 pointerPosition);

	void drawNodeRecursive(UINode* node);

	[[nodiscard]] glm::vec2 screenToLogicalPosition(glm::vec2 screenPosition) const { return screenPosition / getScale(); }

	void setRenderer(IUIRenderer* newRenderer);

	void unregisterNode(UINode* node);

	UINode rootNode;
	UINode* focusedNode = nullptr;
	std::unordered_map<int, UINode*> hoveredNodes;
	std::unordered_map<int, UINode*> downCapturedNodes;
	std::unordered_map<int, UINode*> dragCapturedNodes;

	float dpiScale = 1.f;
	glm::mat4 projectionMatrix{};

	IUIRenderer* activeRenderer = nullptr;
	UIPanelRenderer panelRenderer;
	UITextRenderer textRenderer;
};


#endif // UI_MANAGER_H