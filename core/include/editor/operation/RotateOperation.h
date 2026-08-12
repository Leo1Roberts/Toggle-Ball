#ifndef ROTATE_OPERATION_H
#define ROTATE_OPERATION_H

#include "PivotOperation.h"

class UIText;


class RotateOperation : public PivotOperation {
public:
	RotateOperation(const EditorContext* context, TriggerType trigger, glm::vec2 initialPointerPosition = {})
		: PivotOperation(context, trigger, initialPointerPosition) { RotateOperation::updateDetailsText(); }
	explicit RotateOperation(const TransformOperation& other)
		: PivotOperation(other) { RotateOperation::updateDetailsText(); }

	static std::optional<float> keyToRotationRadians(KeyCode key);

protected:
	void updateDetailsText() override;

private:
	bool doProcessEvent(const Event& event) override;
	void applyOperation() override;

	void setRotation(float radians);

	float rotation = 0.f;
	glm::mat2 rotationMatrix{1.f};
};


#endif // ROTATE_OPERATION_H
