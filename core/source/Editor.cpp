#include "main.h"
#include "Editor.h"

Editor::Editor(LevelDescriptor* levelToEdit) :
	level(levelToEdit),
	ball(levelToEdit->getBallType()) {
	obstacles.append_range(levelToEdit->getObstacleDescriptors()
		| std::views::transform([](const auto& d) { return EditorObstacle(d.get()); }));

	currentNode = makeUndoNode();
}


std::shared_ptr<UndoNode> Editor::makeUndoNode() const {
	return std::make_shared<UndoNode>(level, makeSelectionUndoNode());
}

std::shared_ptr<SelectionUndoNode> Editor::makeSelectionUndoNode() const {
	return std::make_shared<SelectionUndoNode>(
		focus,
		ball.isSelected(),
		obstacles
			| std::views::transform([](const auto& o) { return o.isSelected(); })
			| std::ranges::to<std::vector<bool>>()
	);
}

void Editor::syncLevel() {
	*level = currentNode->level;

	ball = EditorBall(level->getBallType());

	obstacles.assign_range(level->getObstacleDescriptors()
		| std::views::transform([](const auto& d) { return EditorObstacle(d.get()); }));
}

void Editor::syncSelection() {
	focus = currentNode->selection->focus;
	for (size_t i = 0; i < obstacles.size(); i++)
		obstacles[i].setSelected(currentNode->selection->obstacles[i]);
}

void Editor::undo() {
	if (currentNode->selection->previous) { // Undo selection only
		currentNode->selection = currentNode->selection->previous;
		syncSelection();
	} else if (currentNode->previous) { // Undo change to level
		currentNode = currentNode->previous;
		syncLevel();
		syncSelection();
	}
}

void Editor::redo() {
	if (currentNode->selection->next) { // Redo selection only
		currentNode->selection = currentNode->selection->next;
		syncSelection();
	} else if (currentNode->next) { // Redo change to level
		currentNode = currentNode->next;
		syncLevel();
		syncSelection();
	}
}