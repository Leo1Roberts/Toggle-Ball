#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include "UIPanel.h"
#include "UIText.h"

#include <glm/glm.hpp>
#include <unordered_map>


class UIManager {
public:
	void resize(int screenWidth, int screenHeight);

	void changeFocus(UINode* newFocus, bool cancel);

	bool processEvent(const Event& event);

	void setRootNode(std::unique_ptr<UINode> node) { rootNode = std::move(node); }

	void submitPanel(const UIPanel* panel);
	void submitText(const UIText* text);
	void render();

private:
	[[nodiscard]] static UINode* findNodePointedTo(UINode* currentNode, glm::vec2 pointerPosition);

	void drawNodeRecursive(UINode* node);

	void setRenderer(IUIRenderer* newRenderer);

	std::unique_ptr<UINode> rootNode;
	UINode* focusedNode = nullptr;
	std::unordered_map<int, UINode*> hoveredNodes;

	glm::mat4 projectionMatrix = {};

	IUIRenderer* activeRenderer = nullptr;
	UIPanelRenderer panelRenderer;
	UITextRenderer textRenderer;
};


#endif // UI_MANAGER_H
