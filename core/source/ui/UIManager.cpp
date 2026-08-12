#include "ui/UIManager.h"

#include "glm/ext/matrix_clip_space.hpp"

#include <ranges>


void UIManager::resize(int screenWidth, int screenHeight, float screenDPIScale) {
	dpiScale = screenDPIScale;
	logicalScreenSize = glm::vec2((float)screenWidth, (float)screenHeight) / getScale();
	projectionMatrix = glm::ortho(0.f, logicalScreenSize.x, logicalScreenSize.y, 0.f);

	rootNode.updateBounds({
		.position = glm::vec2(0.f),
		.size = logicalScreenSize,
	});
}


bool UIManager::changeFocus(UINode* newFocus, bool cancel) {
	if (focusedNode == newFocus)
		return false;
	if (focusedNode)
		focusedNode->onFocusLost(cancel);

	focusedNode = newFocus;

	if (focusedNode)
		focusedNode->onFocusGained();

	return true;
}


bool UIManager::processEvent(const Event& event) {
	if (std::holds_alternative<KeyEvent>(event) ||
		std::holds_alternative<char>(event)) {
		if (focusedNode) {
			switch (focusedNode->processEvent(event)) {
			case UIResponse::Ignored:
			case UIResponse::ConsumedNeedsHoverUpdate:
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

	if (auto* rawPointer = std::get_if<PointerEvent>(&event)) {
		auto pointer = *rawPointer;

		if (pointer.action == PointerAction::Leave) {
			auto hoverIt = hoveredNodes.find(pointer.id);
			if (hoverIt != hoveredNodes.end()) {
				if (hoverIt->second)
					hoverIt->second->onPointerExited();
				hoveredNodes.erase(hoverIt);
			}

			dragCapturedNodes.erase(pointer.id);
			downCapturedNodes.erase(pointer.id);

			return false;
		}

		pointer.position = screenToLogicalPosition(pointer.position);
		
		UINode* nodePointedTo = nullptr;
		if (rootNode.contains(pointer.position))
			nodePointedTo = findNodePointedTo(&rootNode, pointer.position);


		// Track pointer entry

		UINode* previousHoveredNode = nullptr;
		auto prevHoverIt = hoveredNodes.find(pointer.id);
		if (prevHoverIt != hoveredNodes.end())
			previousHoveredNode = prevHoverIt->second;

		if (previousHoveredNode != nodePointedTo) {
			if (previousHoveredNode)
				previousHoveredNode->onPointerExited();

			if (nodePointedTo) {
				nodePointedTo->onPointerEntered();
				hoveredNodes[pointer.id] = nodePointedTo;
			} else
				hoveredNodes.erase(pointer.id);
		}


		// Allow nodes to capture drag events

		UINode* targetNode = nullptr;

		auto dragIt = dragCapturedNodes.find(pointer.id);
		auto downIt = downCapturedNodes.find(pointer.id);

		if (dragIt != dragCapturedNodes.end() && dragIt->second)
			targetNode = dragIt->second;
		else if (downIt != downCapturedNodes.end() && downIt->second)
			targetNode = downIt->second;
		else {
			targetNode = nodePointedTo;
			if (targetNode && pointer.action == PointerAction::Down)
				downCapturedNodes[pointer.id] = targetNode;
		}


		// Dispatch event to target node

		UIResponse response = UIResponse::Ignored;
		if (targetNode) {
			if (pointer.action == PointerAction::Down && pointer.button == PointerButton::Primary)
				if (changeFocus(targetNode->isFocusable() ? targetNode : nullptr, false))
					pointer.causedFocusChange = true;

			UINode* node = targetNode;
			while (node) {
				response = node->processEvent(pointer);
				if (response == UIResponse::Ignored)
					node = node->getParent();
				else break;
			}

			if (pointer.action == PointerAction::StartDrag && response != UIResponse::Ignored)
				dragCapturedNodes[pointer.id] = node;

			switch (response) {
			case UIResponse::Ignored:
			case UIResponse::Consumed:
				break;
			case UIResponse::ConsumedNeedsHoverUpdate: {
				PointerEvent dummyMove = *rawPointer;
				dummyMove.action = PointerAction::Move;
				this->processEvent(dummyMove);
				break;
			}
			case UIResponse::RequestConfirm:
				changeFocus(nullptr, false);
				break;
			case UIResponse::RequestCancel:
				changeFocus(nullptr, true);
				break;
			}
		} else if (pointer.action == PointerAction::Down) // Pointer down on nothing
			changeFocus(nullptr, false);

		if (pointer.action == PointerAction::Up)
			downCapturedNodes.erase(pointer.id);

		if (pointer.action == PointerAction::FinishDrag || pointer.action == PointerAction::CancelDrag)
			dragCapturedNodes.erase(pointer.id);

		return targetNode != nullptr && response != UIResponse::Ignored;
	}
	return false;
}


UINode* UIManager::findNodePointedTo(UINode* currentNode, glm::vec2 pointerPosition) {
	if (currentNode->childrenAreHitTestable())
		for (const auto& child: std::views::reverse(currentNode->getChildren()))
			if (child->contains(pointerPosition))
				if (auto node = findNodePointedTo(child.get(), pointerPosition)) return node;

	return currentNode->isHitTestable() ? currentNode : nullptr;
}


void UIManager::submitPanel(const UIPanel* panel) {
	setRenderer(&panelRenderer);
	panelRenderer.addPanel(panel);
}
void UIManager::submitText(const UIText* text) {
	setRenderer(&textRenderer);
	textRenderer.addText(text);
}


void UIManager::setRenderer(IUIRenderer* newRenderer) {
	if (activeRenderer != newRenderer) {
		if (activeRenderer)
			activeRenderer->flush(projectionMatrix);
		activeRenderer = newRenderer;

		if (activeRenderer == &textRenderer)
			textRenderer.begin(projectionMatrix);
	}
}

void UIManager::drawNodeRecursive(UINode* node) {
	if (!node->isActive()) return;

	if (node->isVisible())
		node->submitRender(*this);

	for (auto& child : node->getChildren())
		drawNodeRecursive(child.get());
}

void UIManager::render() {
	activeRenderer = nullptr;

	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	drawNodeRecursive(&rootNode);

	if (activeRenderer)
		activeRenderer->flush(projectionMatrix);

	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
}


void UIManager::removeAllChildrenOfNode(UINode* node) {
	for (auto& child : node->getChildren())
		unregisterNode(child.get());

	node->clearChildren();
}
void UIManager::unregisterNode(UINode* node) {
	if (focusedNode == node)
		focusedNode = nullptr;

	auto isNode = [node](const auto& pair) { return pair.second == node; };

	std::erase_if(hoveredNodes, isNode);
	std::erase_if(dragCapturedNodes, isNode);
	std::erase_if(downCapturedNodes, isNode);

	for (const auto& child : node->getChildren())
		unregisterNode(child.get());
}