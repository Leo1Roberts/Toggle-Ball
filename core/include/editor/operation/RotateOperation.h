#ifndef ROTATE_OPERATION_H
#define ROTATE_OPERATION_H

#include "TransformOperation.h"


class RotateOperation : public TransformOperation {
public:
	RotateOperation(const EditorContext* context, TriggerType trigger, glm::vec2 initialPointerPosition = {});

	static std::optional<float> keyToRotationRadians(KeyCode key);

	void renderGizmos() override;

private:
	bool doProcessEvent(const Event& event) override;
	void applyOperation() override;

	void setRotation(float radians);

	glm::vec2 lastPointerPlanarPosition{};

	glm::vec2 pivot{};
	float rotation = 0.f;
	glm::mat2 rotationMatrix{1.f};
};


#endif // ROTATE_OPERATION_H
