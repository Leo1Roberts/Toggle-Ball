#ifndef SELECT_OPERATION_H
#define SELECT_OPERATION_H

#include "EditorScene.h"
#include "Operation.h"


enum class SelectionMode { Replace, Add, Subtract };

class SelectOperation : public Operation {
public:
	SelectOperation(const EditorContext* context, TriggerType trigger, glm::vec2 initialPointerPosition, byte mods, bool instant = false);

	void renderGizmos() override;

private:
	void applyModifiers(byte mods) override;
	bool doProcessEvent(const Event& event) override;
	void applyOperation() override;

	void doCancel() const override { context->scene->cancelSelectionChange(); }
	void doCommit() const override;

	SelectionMode mode = SelectionMode::Replace;
	bool instant;
	SelectBox box{};
};


#endif // SELECT_OPERATION_H
