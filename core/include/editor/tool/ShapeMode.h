#ifndef SHAPE_MODE_H
#define SHAPE_MODE_H

#include "ToolMode.h"


class ShapeMode : public ToolMode {
public:
	explicit ShapeMode(EditorScene& scene, const Camera& camera, const EditorQuickSettings& quickSettings, const float& uiToWorldScale)
		: ToolMode(scene, camera, quickSettings), uiToWorldScale(uiToWorldScale) {}

	void addGizmos(GizmoRenderer& gizmoRenderer) const override;

	[[nodiscard]] std::optional<Cursor> queryCursor() const override;
	
private:
	void performPrimaryAction(const PointerEvent& upEvent) override;
	std::unique_ptr<Operation> startDrag(const PointerEvent& dragStartEvent) override;

	struct CapInfo { int obstacleIndex; bool leftCap; };
	[[nodiscard]] std::optional<CapInfo> getPointedCapInfo(glm::vec2 pointerPlanarPosition) const;
	[[nodiscard]] std::optional<int> getPointedRimIndex(glm::vec2 pointerPlanarPosition) const;

	const float& uiToWorldScale;

	float minorRadius = 0.7f;
};


#endif // SHAPE_MODE_H
