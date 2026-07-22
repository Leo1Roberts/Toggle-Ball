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

	void changeFocus(UINode* newFocus, bool cancel);

	bool processEvent(const Event& event);

	void setRootNode(std::unique_ptr<UINode> node) { rootNode = std::move(node); }

	void submitPanel(const UIPanel* panel);
	void submitText(const UIText* text);
	void render();

private:
	[[nodiscard]] static UINode* findNodePointedTo(UINode* currentNode, glm::vec2 pointerPosition);

	void drawNodeRecursive(UINode* node);

	[[nodiscard]] glm::vec2 screenToLogicalPosition(glm::vec2 screenPosition) const { return screenPosition / getScale(); }

	void setRenderer(IUIRenderer* newRenderer);

	[[nodiscard]] float getScale() const { return dpiScale * Settings::UIScale; }

	std::unique_ptr<UINode> rootNode;
	UINode* focusedNode = nullptr;
	std::unordered_map<int, UINode*> hoveredNodes;
	std::unordered_map<uint32_t, UINode*> capturedNodes;

	float dpiScale = 1.f;
	glm::mat4 projectionMatrix = {};

	IUIRenderer* activeRenderer = nullptr;
	UIPanelRenderer panelRenderer;
	UITextRenderer textRenderer;
};


#endif // UI_MANAGER_H