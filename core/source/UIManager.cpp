#include "UIManager.h"

#include "glm/ext/matrix_clip_space.hpp"

#include <ranges>


void UIManager::resize(int screenWidth, int screenHeight, float screenDPIScale) {
	dpiScale = screenDPIScale;
	glm::vec2 logicalSize = glm::vec2((float)screenWidth, (float)screenHeight) / getScale();
	projectionMatrix = glm::ortho(0.f, logicalSize.x, logicalSize.y, 0.f);

	if (rootNode) {
		Rectangle screenBounds = {
			.x = 0.f, .y = 0.f,
			.width = logicalSize.x, .height = logicalSize.y,
		};
		rootNode->updateLayout(screenBounds);
	}
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

	if (auto* rawPointer = std::get_if<PointerEvent>(&event)) {
		auto pointer = *rawPointer;
		pointer.position = screenToLogicalPosition(pointer.position);
		
		UINode* nodePointedTo = nullptr;

		if (rootNode && rootNode->contains(pointer.position)) {
			nodePointedTo = findNodePointedTo(rootNode.get(), pointer.position);
			if (nodePointedTo == rootNode.get())
				nodePointedTo = nullptr;
		}


		// Track pointer entry

		if (pointer.action == PointerAction::Move || pointer.action == PointerAction::Down) {
			UINode* previousHoveredNode = hoveredNodes[pointer.id];
			if (previousHoveredNode != nodePointedTo) {
				if (previousHoveredNode)
					previousHoveredNode->onPointerExited();
				if (nodePointedTo)
					nodePointedTo->onPointerEntered();
				hoveredNodes[pointer.id] = nodePointedTo;
			}
		}


		// Allow nodes to capture the pointer

		UINode* targetNode = nullptr;
		auto captureIt = capturedNodes.find(pointer.id);

		if (captureIt != capturedNodes.end() && captureIt->second)
			targetNode = captureIt->second; // Continue capturing the pointer
		else {
			targetNode = nodePointedTo;
			if (targetNode && pointer.action == PointerAction::Down)
				capturedNodes[pointer.id] = targetNode; // Start capturing the pointer
		}


		// Dispatch event to target node

		if (targetNode) {
			if (pointer.action == PointerAction::Down && pointer.button == PointerButton::Primary)
				changeFocus(targetNode->isFocusable() ? targetNode : nullptr, false);

			switch (targetNode->processEvent(event)) {
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
		} else if (pointer.action == PointerAction::Down) // Pointer down on nothing
			changeFocus(nullptr, false);


		// Track pointer exit

		if (pointer.action == PointerAction::Up) {
			auto hoverIt = hoveredNodes.find(pointer.id);
			if (hoverIt != hoveredNodes.end()) {
				if (hoverIt->second)
					hoverIt->second->onPointerExited();
				hoveredNodes.erase(hoverIt);
			}

			capturedNodes.erase(pointer.id); // Stop capturing the pointer
		}


		return targetNode != nullptr;
	}
	return false;
}


UINode* UIManager::findNodePointedTo(UINode* currentNode, glm::vec2 pointerPosition) {
	for (const auto& child: std::views::reverse(currentNode->getChildren()))
		if (child->isHitTestable() && child->contains(pointerPosition))
			return findNodePointedTo(child.get(), pointerPosition);

	return currentNode;
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
	if (!rootNode) return;

	activeRenderer = nullptr;

	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	drawNodeRecursive(rootNode.get());

	if (activeRenderer)
		activeRenderer->flush(projectionMatrix);

	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
}