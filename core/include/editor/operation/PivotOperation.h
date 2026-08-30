#ifndef PIVOT_OPERATION_H
#define PIVOT_OPERATION_H

#include "TransformOperation.h"


class PivotOperation : public TransformOperation {
public:
	PivotOperation(EditorScene& scene, const Camera& camera, const EditorQuickSettings& quickSettings, TriggerType trigger, glm::vec2 initialPlanarPosition)
		: TransformOperation(scene, camera, quickSettings, trigger, initialPlanarPosition) { init(); }
	explicit PivotOperation(const TransformOperation& other)
		: TransformOperation(other) { init(); }

	void addGizmos(GizmoRenderer& gizmoRenderer) const override;

protected:
	glm::vec2 pivot{};

private:
	void init();
};


#endif // PIVOT_OPERATION_H
