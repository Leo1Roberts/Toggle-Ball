#include "editor/operation/MinorRadiusOperation.h"

#include "editor/EditorContext.h"


MinorRadiusOperation::MinorRadiusOperation(const EditorContext& ctx, TriggerType trigger, glm::vec2 initialPlanarPosition, float& minorRadius) :
	Operation(ctx, trigger, initialPlanarPosition), minorRadius(minorRadius),
	initialDistance(ctx.scene.obstacles[ctx.scene.selectionFocus.index].getRimProximity(initialPlanarPosition).distance) {}


std::optional<Cursor> MinorRadiusOperation::queryCursor() const {
	auto dir = Camera::planarToScreenDirection(ctx.scene.obstacles[ctx.scene.selectionFocus.index].getRimProximity(pointerPlanarPosition).direction);
	return Cursor{
		.style = Cursor::Style::DynamicResize,
		.dynamic = true,
		.angle = std::atan2(dir.y, dir.x) + glm::half_pi<float>(),
	};
}


OperationResponse MinorRadiusOperation::doProcessEvent(const Event& event) {
	if (auto* pointer = std::get_if<PointerEvent>(&event)) {
		if (pointer->action == PointerAction::Move || pointer->action == PointerAction::Drag) {
			pointerPlanarPosition = ctx.camera.screenToPlanarPosition(pointer->position);
			adjustment = ctx.scene.getCurrentNode()->level.obstacleDescriptors[ctx.scene.selectionFocus.index]->shape->getRimProximity(*ctx.scene.obstacles[ctx.scene.selectionFocus.index].getKinematicState(), pointerPlanarPosition).distance - initialDistance;
			applyOperation();
			return {.consumedEvent = false, .status = OperationStatus::Running};
		}
	}
	return {.consumedEvent = false, .status = OperationStatus::Running};
}


void MinorRadiusOperation::applyOperation() {
	for (int i = 0; i < ctx.scene.obstacles.size(); i++) {
		auto& obstacle = ctx.scene.obstacles[i];
		if (obstacle.isSelected()) {
			obstacle.descriptor->shape->minorRadius =
				ctx.scene.getCurrentNode()->level.obstacleDescriptors[i]->shape->minorRadius + adjustment;
			obstacle.invalidateAllMeshes();
		}
	}
}