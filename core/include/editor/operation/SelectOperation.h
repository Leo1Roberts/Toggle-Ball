#ifndef SELECT_OPERATION_H
#define SELECT_OPERATION_H

#include "editor/EditorScene.h"
#include "Operation.h"


enum class SelectionMode { Replace, Add, Subtract };

class SelectOperation : public Operation {
public:
	SelectOperation(const EditorContext* context, TriggerType trigger, glm::vec2 initialPointerPosition, bool instant = false)
		: Operation(context, trigger, initialPointerPosition), box(initialPointerPlanarPosition), instant(instant) {}

	void renderGizmos() final;

	void finish() const final;

	[[nodiscard]] SelectionMode getMode() const { return mode; }

private:
	void applyModifiers(byte mods) final;
	bool doProcessEvent(const Event& event) final;
	void applyOperation() final;

	void doCancel() const final { context->scene->cancelSelectionChange(); }
	void doCommit() const final { context->scene->commitSelectionChange(); }

	SelectionMode mode = SelectionMode::Replace;
	SelectBox box{};
	bool instant;
};


#endif // SELECT_OPERATION_H
