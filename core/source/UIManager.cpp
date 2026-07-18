#include "UIManager.h"

#include <ranges>


void UIManager::resize(int screenWidth, int screenHeight) {
	Rectangle screenBounds = {
		.x = 0.f, .y = 0.f,
		.width = (float)screenWidth, .height = (float)screenHeight
	};

	if (rootNode)
		rootNode->updateLayout(screenBounds);
}


void UIManager::changeFocus(UINode* newFocus, bool cancel) {
	if (focusedNode == newFocus)
		return;
	if (focusedNode)
		focusedNode->onFocusLost(cancel);

	focusedNode = newFocus;

	if (focusedNode)
		focusedNode->onFocusGained();
}


bool UIManager::processEvent(const Event& event) {
	if (std::holds_alternative<KeyEvent>(event) ||
		std::holds_alternative<CharEvent>(event)) {
		if (focusedNode) {
			switch (focusedNode->processEvent(event)) {
			case UIResponse::Ignored:
				break;
			case UIResponse::Consumed:
				return true;
			case UIResponse::RequestConfirm:
				changeFocus(nullptr, false);
				return true;
			case UIResponse::RequestCancel:
				changeFocus(nullptr, true);
				return true;
			}
		}
		return false;
	}

	if (auto* pointer = std::get_if<PointerEvent>(&event)) {
		if (!rootNode || !rootNode->contains(pointer->position))
			return false;

		UINode* nodePointedTo = findNodePointedTo(rootNode.get(), pointer->position);

		if (nodePointedTo == rootNode.get())
			nodePointedTo = nullptr;

		if (pointer->action == PointerAction::Move || pointer->action == PointerAction::Down) {
			UINode* previousHoveredNode = hoveredNodes[pointer->id];
			if (previousHoveredNode != nodePointedTo) {
				if (previousHoveredNode) previousHoveredNode->onPointerExited();
				if (nodePointedTo) nodePointedTo->onPointerEntered();
				hoveredNodes[pointer->id] = nodePointedTo;
			}
		}

		if (pointer->id > 0 && pointer->action == PointerAction::Up) {
			if (hoveredNodes[pointer->id])
				hoveredNodes[pointer->id]->onPointerExited();
			hoveredNodes.erase(pointer->id);
		}

		if (nodePointedTo) {
			if (pointer->action == PointerAction::Down && pointer->button == PointerButton::Primary)
				changeFocus(nodePointedTo->isFocusable ? nodePointedTo : nullptr, false);

			switch (nodePointedTo->processEvent(event)) {
			case UIResponse::Ignored:
			case UIResponse::Consumed:
				break;
			case UIResponse::RequestConfirm:
				changeFocus(nullptr, false);
				break;
			case UIResponse::RequestCancel:
				changeFocus(nullptr, true);
				break;
			}
			return true;
		}

		// Pointer down on nothing
		if (pointer->action == PointerAction::Down)
			changeFocus(nullptr, false);
	}
	return false;
}


UINode* UIManager::findNodePointedTo(UINode* currentNode, vec2 pointerPosition) {
	for (const auto& child: std::views::reverse(currentNode->children))
		if (child->contains(pointerPosition))
			return findNodePointedTo(child.get(), pointerPosition);

	return currentNode;
}