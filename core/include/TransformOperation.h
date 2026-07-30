#ifndef MOTION_OPERATION_H
#define MOTION_OPERATION_H

#include "EditorScene.h"
#include "Operation.h"


class TransformOperation : public Operation {
public:
	TransformOperation(const EditorContext* context, TriggerType trigger, glm::vec2 initialPointerPosition)
		: Operation(context, trigger, initialPointerPosition) {}

protected:
	bool stateless = false;
	bool local = false;

private:
	void applyModifiers(byte mods) final {
		stateless = mods & MOD_ALT;
		local = mods & MOD_CTRL && mods & MOD_SHIFT;
	}

	[[nodiscard]] bool canStart() const final { return context->scene->anythingIsSelected(); }
	void doCancel() const final { context->scene->cancelLevelChange(); }
	void doCommit() const final { context->scene->commitLevelChange(); }
};


#endif // MOTION_OPERATION_H
