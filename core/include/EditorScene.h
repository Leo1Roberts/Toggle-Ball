#ifndef EDITOR_SCENE_H
#define EDITOR_SCENE_H

#include "Level.h"

#include <memory>
#include <vector>


enum class EntityType { None, Ball, Obstacle };
struct EntityReference {
	EntityType type = EntityType::None;
	unsigned short index = 0;

	bool operator==(const EntityReference&) const = default;
};

struct SelectionState {
	EntityReference focus;
	std::vector<EntityReference> selection;
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
	void commitLevelChange();
	void commitSelectionChange();

	void setSelectionFocus(EntityReference focus);

	[[nodiscard]] const LevelDescriptor* getLevel() const { return level.get(); }
	[[nodiscard]] EditorBall* getBall() { return &ball; }
	[[nodiscard]] std::vector<EditorObstacle>& getObstacles() { return obstacles; }
	[[nodiscard]] float getTogglePosition() const { return togglePosition.getCurrentPosition(); }
	[[nodiscard]] EntityReference getSelectionFocus() const { return selectionFocus; }

private:
	void syncLevel();
	void syncSelection();
	[[nodiscard]] std::shared_ptr<SelectionUndoNode> makeSelectionUndoNode(const std::shared_ptr<SelectionUndoNode>& previous = nullptr) const;

	std::unique_ptr<LevelDescriptor> level;
	EditorBall ball;
	std::vector<EditorObstacle> obstacles;

	bool toggled = false;
	Smoother togglePosition{};

	EntityReference selectionFocus;
	std::shared_ptr<UndoNode> currentNode;
};


#endif // EDITOR_SCENE_H
