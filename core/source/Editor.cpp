#include "main.h"
#include "Editor.h"

#include <ranges>


Editor::Editor(int width, int height) : Screen(width, height) {
	keyBindings = KeyBindings::editor.get();

	// Create UI here
	uiManager.resize(width, height);
}


void Editor::open(const std::shared_ptr<LevelDescriptor>& levelToEdit) {
	level = levelToEdit;
	ball = EditorBall(level->getBallDescriptor().get());
	obstacles.append_range(levelToEdit->getObstacleDescriptors()
		| std::views::transform([](const auto& d) { return EditorObstacle(d.get()); }));

	currentNode = makeUndoNode();
}


std::shared_ptr<UndoNode> Editor::makeUndoNode() const {
	return std::make_shared<UndoNode>(level.get(), makeSelectionUndoNode());
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


void Editor::doProcessEvent(const Event& event) {
	if (uiManager.processEvent(event))
		return;

	if (auto* key = std::get_if<KeyEvent>(&event)) {
		if (auto actionCode = keyBindings->translate(key->code)) {
			if (key->action == KeyAction::Down) {
				switch (*actionCode) {
				default:;
				}
			}
		}
	}
}

void Editor::doUpdate(microseconds dt) {

}

void Editor::doDraw() {
	glClearColor(0.5, 0.5, 0.5, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


void Editor::syncLevel() {
	*level = currentNode->level;

	ball = EditorBall(level->getBallDescriptor().get());

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