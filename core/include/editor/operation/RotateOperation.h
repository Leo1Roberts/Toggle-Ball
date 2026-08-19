#ifndef ROTATE_OPERATION_H
#define ROTATE_OPERATION_H

#include "PivotOperation.h"

class UIText;


class RotateOperation : public PivotOperation {
public:
	RotateOperation(const EditorContext* context, TriggerType trigger, glm::vec2 initialPointerPosition = {})
		: PivotOperation(context, trigger, initialPointerPosition, ActionCode::Rotate) { RotateOperation::createUI(); }
	explicit RotateOperation(const TransformOperation& other)
		: PivotOperation(other, ActionCode::Rotate) { RotateOperation::createUI(); }

	static std::optional<float> keyToRotationRadians(KeyCode key);

protected:
	void createUI() override;
	void updateDetailsText() override;

	bool doProcessEvent(const Event& event) override;
	void applyOperation() override;

private:
	void setRotation(float radians);
	static float angleDifference(glm::vec2 newPos, glm::vec2 oldPos, glm::vec2 pivot);
	void updateTransformation(glm::vec2 newPointerPlanarPosition) override {
		setRotation(rotation + angleDifference(newPointerPlanarPosition, pointerPlanarPosition, pivot) * precisionMultiplier);
	}

	float rotation = 0.f;
	glm::mat2 rotationMatrix{1.f};
};


#endif // ROTATE_OPERATION_H
