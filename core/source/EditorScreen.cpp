#include "main.h"
#include "EditorScreen.h"

#include "Settings.h"

#include <ranges>


EditorScreen::EditorScreen() {
	// Create UI here
}


void EditorScreen::open(const std::shared_ptr<LevelDescriptor>& levelToEdit) {
	level = levelToEdit;
	ball = EditorBall(level->getBallDescriptor().get());
	obstacles.append_range(levelToEdit->getObstacleDescriptors()
		| std::views::transform([](const auto& d) { return EditorObstacle(d.get()); }));

	currentNode = makeUndoNode();
}


std::shared_ptr<UndoNode> EditorScreen::makeUndoNode() const {
	return std::make_shared<UndoNode>(level.get(), makeSelectionUndoNode());
}

std::shared_ptr<SelectionUndoNode> EditorScreen::makeSelectionUndoNode() const {
	return std::make_shared<SelectionUndoNode>(
		focus,
		ball.isSelected(),
		obstacles
			| std::views::transform([](const auto& o) { return o.isSelected(); })
			| std::ranges::to<std::vector<bool>>()
	);
}


void EditorScreen::processEvent(const Event& event) {
	if (uiManager.processEvent(event))
		return;

	if (auto* key = std::get_if<KeyEvent>(&event)) {
		if (auto actionCode = Settings::Bindings->translate(key->chord)) {
			if (key->action == KeyAction::Down) {
				switch (*actionCode) {
				case ActionCode::Undo:
					undo();
				case ActionCode::Redo:
					redo();
				default:;
				}
			}
		}
	}
}

void EditorScreen::update(microseconds dt) {

}

void EditorScreen::render() {

}


void EditorScreen::syncLevel() {
	*level = currentNode->level;

	ball = EditorBall(level->getBallDescriptor().get());

	obstacles.assign_range(level->getObstacleDescriptors()
		| std::views::transform([](const auto& d) { return EditorObstacle(d.get()); }));
}

void EditorScreen::syncSelection() {
	focus = currentNode->selection->focus;
	for (size_t i = 0; i < obstacles.size(); i++)
		obstacles[i].setSelected(currentNode->selection->obstacles[i]);
}

void EditorScreen::undo() {
	if (currentNode->selection->previous) { // Undo selection only
		currentNode->selection = currentNode->selection->previous;
		syncSelection();
	} else if (currentNode->previous) { // Undo change to level
		currentNode = currentNode->previous;
		syncLevel();
		syncSelection();
	}
}

void EditorScreen::redo() {
	if (currentNode->selection->next) { // Redo selection only
		currentNode->selection = currentNode->selection->next;
		syncSelection();
	} else if (currentNode->next) { // Redo change to level
		currentNode = currentNode->next;
		syncLevel();
		syncSelection();
	}
}