#ifndef PIVOT_OPERATION_H
#define PIVOT_OPERATION_H

#include "TransformOperation.h"


class PivotOperation : public TransformOperation {
public:
	PivotOperation(const EditorContext& ctx, TriggerType trigger, glm::vec2 initialPlanarPosition)
		: TransformOperation(ctx, trigger, initialPlanarPosition) { init(); }
	explicit PivotOperation(const TransformOperation& other)
		: TransformOperation(other) { init(); }

	void addGizmos(GizmoRenderer& gizmoRenderer) const override;

protected:
	glm::vec2 pivot{};

private:
	void init();
};


#endif // PIVOT_OPERATION_H
