#include "EditorScene.h"

#include <ranges>


EditorScene::EditorScene(std::unique_ptr<LevelDescriptor> levelToEdit) {
	level = std::move(levelToEdit);
	ball = EditorBall(level->getBallDescriptor().get());
	obstacles.append_range(level->getObstacleDescriptors()
		| std::views::transform([](const auto& d) { return EditorObstacle(d.get()); }));

	currentNode = std::make_shared<UndoNode>(*level, makeSelectionUndoNode());
}


void EditorScene::update(microseconds dt) {
	togglePosition.update(toSeconds(dt));
	for (auto& obstacle: obstacles)
		obstacle.updateKinematicState(togglePosition);
}


void EditorScene::toggle(bool transition) {
	toggled = !toggled;
	if (transition)
		togglePosition.setDestination(toggled, 0.f, level->getTransitionTime());
	else
		togglePosition.setPosition(toggled);
}


void EditorScene::undo() {
	if (currentNode->selectionNode->previous) { // Undo selection only
		currentNode->selectionNode = currentNode->selectionNode->previous;
		syncSelection();
	} else if (currentNode->previous) { // Undo change to level
		currentNode = currentNode->previous;
		syncLevel();
		syncSelection();
	}
}
void EditorScene::redo() {
	if (currentNode->selectionNode->next) { // Redo selection only
		currentNode->selectionNode = currentNode->selectionNode->next;
		syncSelection();
	} else if (currentNode->next) { // Redo change to level
		currentNode = currentNode->next;
		syncLevel();
		syncSelection();
	}
}

void EditorScene::cancelLevelChange() {
	syncLevel();
}
void EditorScene::cancelSelectionChange() {
	syncSelection();
}
void EditorScene::commitLevelChange() { // TODO: only commit if actually changed
	currentNode = std::make_shared<UndoNode>(*level, makeSelectionUndoNode(), currentNode);
	currentNode->previous->next = currentNode;
}
void EditorScene::commitSelectionChange() { // TODO: only commit if actually changed
	currentNode->selectionNode = makeSelectionUndoNode(currentNode->selectionNode);
	currentNode->selectionNode->previous->next = currentNode->selectionNode;
}

void EditorScene::syncLevel() { // TODO: generate ephemeral meshes
	*level = currentNode->level;

	ball = EditorBall(level->getBallDescriptor().get());

	obstacles.assign_range(level->getObstacleDescriptors()
		| std::views::transform([](const auto& d) { return EditorObstacle(d.get()); }));


}
void EditorScene::syncSelection() {
	const auto& selection = currentNode->selectionNode->selectionState;
	selectionFocus = selection.focus;
	ball.setSelected(selection.ball);
	for (int i = 0; i < obstacles.size(); i++)
		obstacles[i].setSelected(selection.obstacles[i]);
}

std::shared_ptr<SelectionUndoNode> EditorScene::makeSelectionUndoNode(const std::shared_ptr<SelectionUndoNode>& previous) const {
	return std::make_shared<SelectionUndoNode>(SelectionState(
		selectionFocus,
		ball.isSelected(),
		obstacles
			| std::views::transform([](const auto& o) { return o.isSelected(); })
			| std::ranges::to<std::vector<bool>>()),
		previous);
}


void EditorScene::setSelectionFocus(EntityReference focus) {
	selectionFocus = focus;
}