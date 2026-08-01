#ifndef EDITOR_SCENE_H
#define EDITOR_SCENE_H

#include "Level.h"

#include <memory>
#include <vector>


enum class EntityType { None, Ball, Obstacle };
struct EntityReference {
	EntityType type = EntityType::None;
	int index = 0;

	bool operator==(const EntityReference&) const = default;
};

struct SelectionState {
	EntityReference focus;
	bool ball;
	std::vector<bool> obstacles;

	bool operator==(const SelectionState&) const = default;
};


struct SelectionUndoNode {
	SelectionUndoNode(SelectionState selectionState, const std::shared_ptr<SelectionUndoNode>& previous = nullptr) :
		selectionState(std::move(selectionState)), previous(previous) {}

	SelectionState selectionState;

	std::shared_ptr<SelectionUndoNode> previous;
	std::shared_ptr<SelectionUndoNode> next;
};

struct UndoNode {
	UndoNode(const LevelDescriptor& level, const std::shared_ptr<SelectionUndoNode>& selectionNode, const std::shared_ptr<UndoNode>& previous = nullptr) :
		level(level), selectionNode(selectionNode), previous(previous) {}

	LevelDescriptor level;
	std::shared_ptr<SelectionUndoNode> selectionNode;

	std::shared_ptr<UndoNode> previous;
	std::shared_ptr<UndoNode> next;
};


class EditorScene {
public:
	EditorScene(std::unique_ptr<LevelDescriptor> levelToEdit, const std::function<void()>& syncLevelCallback);

	void update(microseconds dt);
	void toggle(bool transition = true);

	void undo();
	void redo();
	void cancelLevelChange();
	void cancelSelectionChange();
	void commitLevelChange();
	void commitSelectionChange();

	void setSelectionFocus(EntityReference focus);
	void selectAll();
	void deselectAll();
	[[nodiscard]] bool anythingIsSelected() const { return selectionFocus.type != EntityType::None; }

	[[nodiscard]] bool isToggled() const { return toggled; }
	[[nodiscard]] float getTogglePosition() const { return togglePosition.getCurrentPosition(); }
	[[nodiscard]] UndoNode* getCurrentNode() const { return currentNode.get(); }

	std::unique_ptr<LevelDescriptor> level;
	EditorBall ball;
	std::vector<EditorObstacle> obstacles;

	EntityReference selectionFocus = {EntityType::None};

private:
	void syncLevel();
	std::function<void()> syncLevelCallback;
	void syncSelection();
	[[nodiscard]] SelectionState getSelectionState() const;

	bool toggled = false;
	Smoother togglePosition{};

	std::shared_ptr<UndoNode> currentNode;
};


#endif // EDITOR_SCENE_H
