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
	explicit EditorScene(std::unique_ptr<LevelDescriptor> levelToEdit);

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

	[[nodiscard]] const LevelDescriptor* getLevel() const { return level.get(); }
	[[nodiscard]] EditorBall* getBall() { return &ball; }
	[[nodiscard]] std::vector<EditorObstacle>& getObstacles() { return obstacles; }
	[[nodiscard]] float getTogglePosition() const { return togglePosition.getCurrentPosition(); }
	[[nodiscard]] EntityReference* getSelectionFocus() { return &selectionFocus; }
	[[nodiscard]] UndoNode* getCurrentNode() const { return currentNode.get(); }

private:
	void syncLevel();
	void syncSelection();
	[[nodiscard]] std::shared_ptr<SelectionUndoNode> makeSelectionUndoNode(const std::shared_ptr<SelectionUndoNode>& previous = nullptr) const;

	std::unique_ptr<LevelDescriptor> level;
	EditorBall ball;
	std::vector<EditorObstacle> obstacles;

	EntityReference selectionFocus = {EntityType::None};

	bool toggled = false;
	Smoother togglePosition{};

	std::shared_ptr<UndoNode> currentNode;
};


#endif // EDITOR_SCENE_H
