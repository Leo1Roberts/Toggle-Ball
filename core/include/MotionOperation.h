#ifndef MOTION_OPERATION_H
#define MOTION_OPERATION_H

#include "EditorScene.h"
#include "Operation.h"


class MotionOperation : public Operation {
public:
	MotionOperation(const EditorContext* context, TriggerType trigger, glm::vec2 initialPointerPosition, byte mods)
		: Operation(context, trigger, initialPointerPosition) {
		applyModifiers(mods);
	}

private:
	void applyModifiers(byte mods) final { stateless = mods & MOD_ALT; }

	void doCancel() const final { context->scene->cancelLevelChange(); }
	void doCommit() const final { context->scene->commitLevelChange(); }

	bool stateless = false;
};


#endif // MOTION_OPERATION_H
