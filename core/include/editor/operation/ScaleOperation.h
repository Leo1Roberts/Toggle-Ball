#ifndef SCALE_OPERATION_H
#define SCALE_OPERATION_H

#include "TransformOperation.h"


class ScaleOperation : public TransformOperation {
public:
	ScaleOperation(const EditorContext* context, TriggerType trigger, glm::vec2 initialPointerPosition = {});

	void renderGizmos() override;

private:
	bool doProcessEvent(const Event& event) override;
	void applyOperation() override;

	glm::vec2 pointerPlanarPosition{};

	glm::vec2 pivot{};
	float scale = 1.f;
};


#endif // SCALE_OPERATION_H
