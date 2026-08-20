#ifndef PIVOT_OPERATION_H
#define PIVOT_OPERATION_H

#include "TransformOperation.h"


class PivotOperation : public TransformOperation {
public:
	PivotOperation(EditorScene* scene, const Camera* camera, const TransformQuickSettings& settings, TriggerType trigger, glm::vec2 initialPlanarPosition)
		: TransformOperation(scene, camera, settings, trigger, initialPlanarPosition) { init(); }
	PivotOperation(const TransformOperation& other)
		: TransformOperation(other) { init(); }

	void renderGizmos(GizmoRenderer& gizmoRenderer) override;

protected:
	glm::vec2 pivot{};

private:
	void init();
};


#endif // PIVOT_OPERATION_H
