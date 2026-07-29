#ifndef SELECT_OPERATION_H
#define SELECT_OPERATION_H

#include "EditorScene.h"
#include "Operation.h"


enum class SelectionMode { Replace, Add, Subtract };

class SelectOperation : public Operation {
public:
	SelectOperation(const EditorContext* context, TriggerType trigger, glm::vec2 initialPointerPosition, byte mods, bool instant = false);

	void renderGizmos() final;

	void finish() const final;

private:
	void applyModifiers(byte mods) final;
	bool doProcessEvent(const Event& event) final;
	void applyOperation() final;

	void doCancel() const final { context->scene->cancelSelectionChange(); }
	void doCommit() const final { context->scene->commitSelectionChange(); }

	SelectionMode mode = SelectionMode::Replace;
	bool instant;
	SelectBox box{};
};


#endif // SELECT_OPERATION_H
