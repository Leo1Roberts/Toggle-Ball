#ifndef PIVOT_OPERATION_H
#define PIVOT_OPERATION_H

#include "TransformOperation.h"


class PivotOperation : public TransformOperation {
public:
	PivotOperation(const EditorContext* context, TriggerType trigger, glm::vec2 initialPointerPosition = {})
		: TransformOperation(context, trigger, initialPointerPosition) { init(); }
	explicit PivotOperation(const TransformOperation& other)
		: TransformOperation(other) { init(); }

	void renderGizmos() override;

protected:
	glm::vec2 pointerPlanarPosition{};

	glm::vec2 pivot{};

private:
	void init();
};


#endif // PIVOT_OPERATION_H
