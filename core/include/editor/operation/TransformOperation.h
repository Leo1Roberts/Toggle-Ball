#ifndef TRANSFORM_OPERATION_H
#define TRANSFORM_OPERATION_H

#include "editor/EditorScene.h"
#include "Operation.h"
#include "io/TextInputBuffer.h"

class UIText;


class TransformOperation : public Operation {
public:
	TransformOperation(const EditorContext* context, TriggerType trigger, glm::vec2 initialPointerPosition)
		: Operation(context, trigger, initialPointerPosition), pointerPlanarPosition(initialPointerPlanarPosition) {
		TransformOperation::createUI();
	}
	TransformOperation(const TransformOperation& other) // Reset typing
		: Operation(other), pointerPlanarPosition(initialPointerPlanarPosition) {
		TransformOperation::createUI();
	}

protected:
	void createUI() override;
	virtual void updateDetailsText() = 0;

	glm::vec2 pointerPlanarPosition;

	float precisionMultiplier = 1.f;

	bool typing = false;
	TextInputBuffer textInput{TextInputBuffer::Float, TextInputMode::Simple};

	UIText* detailsText = nullptr;


private:
	void applyModifiers(byte mods) final {
		precisionMultiplier = mods & MOD_SHIFT ? 0.1f : 1.f;
	}

	[[nodiscard]] bool canStart() const final { return context->scene->anythingIsSelected(); }
	void doCancel() const final { context->scene->cancelLevelChange(); }
	void doCommit() const final { context->scene->commitLevelChange(); }
};


#endif // TRANSFORM_OPERATION_H
