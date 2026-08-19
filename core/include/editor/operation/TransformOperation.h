#ifndef TRANSFORM_OPERATION_H
#define TRANSFORM_OPERATION_H

#include "editor/EditorScene.h"
#include "Operation.h"
#include "io/TextInputBuffer.h"

class UIText;


class TransformOperation : public Operation {
public:
	TransformOperation(const EditorContext* context, TriggerType trigger, glm::vec2 initialPointerPosition, ActionCode code)
		: Operation(context, trigger, initialPointerPosition), associatedCode(code), pointerPlanarPosition(initialPointerPlanarPosition) {
		TransformOperation::createUI();
	}
	TransformOperation(const TransformOperation&) = delete;
	TransformOperation(const TransformOperation& other, ActionCode code) // Reset typing
		: Operation(other), associatedCode(code), pointerPlanarPosition(initialPointerPlanarPosition) {
		TransformOperation::createUI();
	}

	const ActionCode associatedCode;

protected:
	void createUI() override;
	virtual void updateDetailsText() = 0;

	bool doProcessEvent(const Event& event) override;

	glm::vec2 pointerPlanarPosition;

	float precisionMultiplier = 1.f;

	bool typing = false;
	TextInputBuffer textInput{TextInputBuffer::Float, TextInputMode::Simple};

	UIText* detailsText = nullptr;

private:
	virtual void updateTransformation(glm::vec2 newPointerPlanarPosition) = 0;

	void applyModifiers(byte mods) final {
		precisionMultiplier = mods & MOD_SHIFT ? 0.1f : 1.f;
	}

	[[nodiscard]] bool canStart() const final { return context->scene->anythingIsSelected(); }
	void doCancel() const final { context->scene->cancelLevelChange(); }
	void doCommit() const final { context->scene->commitLevelChange(); }
};


#endif // TRANSFORM_OPERATION_H
