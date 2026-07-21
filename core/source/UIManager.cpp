#include "UIManager.h"

#include "glm/ext/matrix_clip_space.hpp"

#include <ranges>


void UIManager::resize(int screenWidth, int screenHeight) {
	Rectangle screenBounds = {
		.x = 0.f, .y = 0.f,
		.width = (float)screenWidth, .height = (float)screenHeight
	};

	projectionMatrix = glm::ortho(0.f, (float)screenWidth, (float)screenHeight, 0.f);

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
				changeFocus(nodePointedTo->isFocusable() ? nodePointedTo : nullptr, false);

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


UINode* UIManager::findNodePointedTo(UINode* currentNode, glm::vec2 pointerPosition) {
	for (const auto& child: std::views::reverse(currentNode->getChildren()))
		if (child->contains(pointerPosition))
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