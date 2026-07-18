#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include "UINode.h"

#include <unordered_map>


class UIManager {
public:
	void resize(int screenWidth, int screenHeight);

	void changeFocus(UINode* newFocus, bool cancel);

	bool processEvent(const Event& event);

	void setRootNode(std::unique_ptr<UINode> node) { rootNode = std::move(node); }

private:
	[[nodiscard]] static UINode* findNodePointedTo(UINode* currentNode, vec2 pointerPosition);

	std::unique_ptr<UINode> rootNode;
	UINode* focusedNode{nullptr};
	std::unordered_map<int, UINode*> hoveredNodes;
};


#endif // UI_MANAGER_H
