#ifndef PIVOT_OPERATION_H
#define PIVOT_OPERATION_H

#include "TransformOperation.h"


class PivotOperation : public TransformOperation {
public:
	PivotOperation(const EditorContext* context, TriggerType trigger, glm::vec2 initialPointerPosition, ActionCode code)
		: TransformOperation(context, trigger, initialPointerPosition, code) { init(); }
	PivotOperation(const TransformOperation& other, ActionCode code)
		: TransformOperation(other, code) { init(); }

	void renderGizmos() override;

protected:
	glm::vec2 pivot{};

private:
	void init();
};


#endif // PIVOT_OPERATION_H
