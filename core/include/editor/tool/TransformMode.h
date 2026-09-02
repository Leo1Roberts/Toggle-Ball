#ifndef TRANSFORM_MODE_H
#define TRANSFORM_MODE_H

#include "ToolMode.h"


class TransformMode : public ToolMode {
public:
	explicit TransformMode(const EditorContext& ctx) : ToolMode(ctx) {}

	void addGizmos(GizmoRenderer& gizmoRenderer) const override;

protected:
	std::unique_ptr<Operation> startDrag(const PointerEvent& dragStartEvent) override;
};


#endif // TRANSFORM_MODE_H
