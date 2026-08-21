#include "editor/EditorScene.h"

#include <ranges>


EditorScene::EditorScene(std::unique_ptr<LevelDescriptor> levelToEdit, const std::function<void()>& syncLevelCallback)
	: level(std::move(levelToEdit)), ball(level->ballDescriptor.get()), syncLevelCallback(syncLevelCallback) {

	obstacles.append_range(level->obstacleDescriptors
		| std::views::transform([](const auto& d) { return EditorObstacle(d.get()); }));
	deselectAll();

	currentNode = std::make_shared<UndoNode>(*level, std::make_shared<SelectionUndoNode>(getSelectionState()));
}


void EditorScene::update(microseconds dt) {
	float dt_s = toSeconds(dt);
	togglePosition.update(dt_s);
	for (auto& obstacle: obstacles)
		obstacle.updateKinematicState(togglePosition,
			demonstrateMotion && obstacle.isSelected() ?
			(int)(dt_s / PHYSICS_TIMESTEP) : -1);
}


void EditorScene::toggle(bool transition) {
	toggled = !toggled;
	if (transition)
		togglePosition.setDestination(toggled, 0.f, level->transitionTime);
	else
		togglePosition.setPosition(toggled);
}


bool EditorScene::copySelection(std::vector<ObstacleDescriptor>* target) {
	if (!target) target = &clipboard;
	bool anythingIsSelected = false;
	for (const auto& obstacle: obstacles)
		if (obstacle.isSelected()) {
			if (!anythingIsSelected) {
				target->clear();
				anythingIsSelected = true;
			}
			target->emplace_back(*obstacle.descriptor);
		}
	return anythingIsSelected;
}
void EditorScene::deleteSelection() {
	int writeIndex = 0;
	for (int readIndex = 0; readIndex < obstacles.size(); readIndex++)
		if (!obstacles[readIndex].isSelected()) {
			if (writeIndex != readIndex) {
				level->obstacleDescriptors[writeIndex] = std::move(level->obstacleDescriptors[readIndex]);
				obstacles[writeIndex] = std::move(obstacles[readIndex]);
			}
			writeIndex++;
		}
	level->obstacleDescriptors.erase(level->obstacleDescriptors.begin() + writeIndex, level->obstacleDescriptors.end());
	obstacles.erase(obstacles.begin() + writeIndex, obstacles.end());

	deselectAll();

	commitLevelChange();
}
bool EditorScene::paste(std::vector<ObstacleDescriptor>* source) {
	if (!source) source = &clipboard;
	if (source->size() == 0) return false;
	int originalObstaclesCount = level->obstacleDescriptors.size();

	level->obstacleDescriptors.append_range(
		*source | std::views::transform([](const auto& clipboardObstacle) {
			return std::make_unique<ObstacleDescriptor>(clipboardObstacle);
		})
	);

	deselectAll();
	obstacles.append_range(std::span(level->obstacleDescriptors).subspan(originalObstaclesCount)
		| std::views::transform([](const auto& d) { return EditorObstacle(d.get()); }));
	syncLevelCallback();

	for (int i = (int)obstacles.size() - 1; i >= 0; i--)
		if (obstacles[i].isSelected()) {
			selectionFocus = {EntityType::Obstacle, i};
			break;
		}

	commitLevelChange();

	return true;
}
bool EditorScene::duplicateSelection() {
	std::vector<ObstacleDescriptor> temporaryClipboard;
	if (copySelection(&temporaryClipboard)) {
		paste(&temporaryClipboard);
		return true;
	}
	return false;
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
	syncSelection();
}
void EditorScene::cancelSelectionChange() {
	syncSelection();
}
void EditorScene::commitLevelChange() {
	if (*level != currentNode->level) {
		currentNode = std::make_shared<UndoNode>(*level, std::make_shared<SelectionUndoNode>(getSelectionState()), currentNode);
		currentNode->previous->next = currentNode;
	}
}
void EditorScene::commitSelectionChange() {
	if (currentNode->selectionNode) {
		SelectionState newSelection = getSelectionState();
		if (currentNode->selectionNode->selectionState != newSelection) {
			currentNode->selectionNode = std::make_shared<SelectionUndoNode>(newSelection, currentNode->selectionNode);
			currentNode->selectionNode->previous->next = currentNode->selectionNode;
		}
	} else
		currentNode->selectionNode = std::make_shared<SelectionUndoNode>(getSelectionState());
}

void EditorScene::syncLevel() {
	*level = currentNode->level;

	ball = EditorBall(level->ballDescriptor.get());

	obstacles.assign_range(level->obstacleDescriptors
		| std::views::transform([](const auto& d) { return EditorObstacle(d.get()); }));

	syncLevelCallback();

	syncSelection();
}
void EditorScene::syncSelection() {
	const auto& selection = currentNode->selectionNode->selectionState;
	selectionFocus = selection.focus;
	ball.setSelected(selection.ball);
	for (int i = 0; i < obstacles.size(); i++)
		obstacles[i].setSelected(selection.obstacles[i]);
}

bool EditorScene::anythingIsSelected() const {
	return
	ball.isSelected() ||
	std::ranges::any_of(obstacles, &EditorObstacle::isSelected);
}

SelectionState EditorScene::getSelectionState() const {
	return {
		selectionFocus,
		ball.isSelected(),
		obstacles
			| std::views::transform(&EditorObstacle::isSelected)
			| std::ranges::to<std::vector<bool>>()
	};
}
void EditorScene::applySelectionState(const SelectionState& state) {
	selectionFocus = state.focus;
	ball.setSelected(state.ball);
	for (int i = 0; i < obstacles.size(); i++)
		obstacles[i].setSelected(state.obstacles[i]);
}


void EditorScene::selectAll() {
	ball.select();
	for (auto& obstacle: obstacles)
		obstacle.select();
	if (selectionFocus.type == EntityType::None)
		selectionFocus = {EntityType::Ball};
}
void EditorScene::deselectAll() {
	ball.deselect();
	for (auto& obstacle: obstacles)
		obstacle.deselect();
	selectionFocus = {};
}