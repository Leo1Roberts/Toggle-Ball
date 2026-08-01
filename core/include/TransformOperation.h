#ifndef TRANSFORM_OPERATION_H
#define TRANSFORM_OPERATION_H

#include "EditorScene.h"
#include "Operation.h"
#include "TextInputBuffer.h"


class TransformOperation : public Operation {
public:
	TransformOperation(const EditorContext* context, TriggerType trigger, glm::vec2 initialPointerPosition)
		: Operation(context, trigger, initialPointerPosition) {}

protected:
	bool typing = false;
	TextInputBuffer textInput{TextInputBuffer::Float, TextInputMode::Simple};

private:
	void applyModifiers(byte mods) final {}

	[[nodiscard]] bool canStart() const final { return context->scene->anythingIsSelected(); }
	void doCancel() const final { context->scene->cancelLevelChange(); }
	void doCommit() const final { context->scene->commitLevelChange(); }
};


#endif // TRANSFORM_OPERATION_H
