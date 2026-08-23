#ifndef TRANSFORM_MODE_H
#define TRANSFORM_MODE_H

#include "ToolMode.h"
#include "editor/operation/TransformOperation.h"


class TransformMode : public ToolMode {
public:
	explicit TransformMode(EditorScene* scene, const Camera* camera)
		: ToolMode(scene, camera) {}

	[[nodiscard]] std::vector<BindingHint> getBindingHints() const override;
	void populateToolbar(UINode& toolbar) override;
	void renderGizmos(GizmoRenderer& gizmoRenderer) override;

private:
	[[nodiscard]] ToolModeResponse doProcessEvent(const Event& event) override;

	std::unique_ptr<Operation> startDrag(const PointerEvent& dragStartEvent) override;

	TransformQuickSettings settings;
};


#endif // TRANSFORM_MODE_H
