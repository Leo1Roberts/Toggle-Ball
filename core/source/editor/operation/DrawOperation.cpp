#include "editor/operation/DrawOperation.h"

#include "editor/EditorContext.h"
#include "editor/operation/ManipulateCapOperation.h"


DrawOperation::DrawOperation(const EditorContext& ctx, TriggerType trigger, glm::vec2 initialPlanarPosition, float minorRadius, std::optional<float> tangentAngle)
	: Operation(ctx, trigger, initialPlanarPosition) {
	ctx.scene.deselectAll();
	auto shape = std::make_unique<SegmentSpec>(minorRadius, 0.f, 0.f);
	auto motion = std::make_unique<StaticSpec>(initialPlanarPosition, 0.f);
	ctx.scene.selectionFocus = {EntityType::Obstacle, (int)ctx.scene.obstacles.size() - 1};
	manipulateCapOperation = std::make_unique<ManipulateRightCapOperation>(ctx, trigger, initialPlanarPosition, ctx.scene.obstacles.size() - 1, tangentAngle);
}