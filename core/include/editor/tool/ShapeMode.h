#ifndef SHAPE_MODE_H
#define SHAPE_MODE_H

#include "ToolMode.h"


class ShapeMode : public ToolMode {
public:
	explicit ShapeMode(const EditorContext& ctx) : ToolMode(ctx) {}

	void addGizmos(GizmoRenderer& gizmoRenderer) const override;

	[[nodiscard]] std::optional<Cursor> queryCursor() const override;

	float minorRadius = 0.7f;

protected:
	void performPrimaryAction(const PointerEvent& upEvent) override;
	std::unique_ptr<Operation> startDrag(const PointerEvent& dragStartEvent) override;

private:
	struct CapInfo { int obstacleIndex; bool leftCap; };
	[[nodiscard]] std::optional<CapInfo> getPointedCapHandleInfo(glm::vec2 pointerPlanarPosition) const;

	struct MidsectionHandleInfo { bool pointed; glm::vec2 position; float angle; };
	[[nodiscard]] std::optional<MidsectionHandleInfo> getMidsectionHandleInfo(const EditorObstacle& obstacle, glm::vec2 pointerPlanarPosition) const;

	[[nodiscard]] std::optional<int> getPointedRimIndex(glm::vec2 pointerPlanarPosition) const;
};


#endif // SHAPE_MODE_H
