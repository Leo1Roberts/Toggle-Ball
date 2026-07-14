#ifndef EDITOR_H
#define EDITOR_H

#include "Obstacle.h"
#include "Level.h"
#include "Screen.h"

struct SelectionUndoNode {
	short focus;
	bool ball;
	std::vector<bool> obstacles;

	std::shared_ptr<SelectionUndoNode> previous;
	std::shared_ptr<SelectionUndoNode> next;
};

struct UndoNode {
	UndoNode(const LevelDescriptor* level, const std::shared_ptr<SelectionUndoNode>& selection, const std::shared_ptr<UndoNode>& previous = nullptr, const std::shared_ptr<UndoNode>& next = nullptr) :
		level(*level), selection(selection), previous(previous), next(next) {}

	LevelDescriptor level;
	std::shared_ptr<SelectionUndoNode> selection;

	std::shared_ptr<UndoNode> previous;
	std::shared_ptr<UndoNode> next;
};

class Editor : public Screen {
public:
	Editor(int width, int height) : Screen(width, height) {}

	void open(const std::shared_ptr<LevelDescriptor>& levelToEdit);

private:
	bool doProcessEvent(const Event&) override;
	void doUpdate(float dt) override;
	void doDraw() override;

	[[nodiscard]] std::shared_ptr<UndoNode> makeUndoNode() const;
	[[nodiscard]] std::shared_ptr<SelectionUndoNode> makeSelectionUndoNode() const;
	void syncLevel();
	void syncSelection();
	void undo();
	void redo();

	std::shared_ptr<LevelDescriptor> level{};

	EditorBall ball{};
	std::vector<EditorObstacle> obstacles;
	short focus{}; // TODO: decide how value is interpreted (no focus, ball focus, obstacle focus)
	std::shared_ptr<UndoNode> currentNode;

	Smoother togglePosition{};
};

#endif // EDITOR_H
