#include "editor/operation/MinorRadiusOperation.h"


std::optional<Cursor> MinorRadiusOperation::queryCursor() const {
	auto dir = Camera::planarToScreenDirection(scene.obstacles[scene.selectionFocus.index].getRimProximity(pointerPlanarPosition).direction);
	return Cursor{
		.style = Cursor::Style::DynamicResize,
		.dynamic = true,
		.angle = std::atan2(dir.y, dir.x) + glm::half_pi<float>(),
	};
}


OperationResponse MinorRadiusOperation::doProcessEvent(const Event& event) {
	if (auto* pointer = std::get_if<PointerEvent>(&event)) {
		if (pointer->action == PointerAction::Move || pointer->action == PointerAction::Drag) {
			pointerPlanarPosition = camera.screenToPlanarPosition(pointer->position);
			adjustment = scene.getCurrentNode()->level.obstacleDescriptors[scene.selectionFocus.index]->shape->getRimProximity(*scene.obstacles[scene.selectionFocus.index].getKinematicState(), pointerPlanarPosition).distance - initialDistance;
			applyOperation();
			return {.consumedEvent = false, .status = OperationStatus::Running};
		}
	}
	return {.consumedEvent = false, .status = OperationStatus::Running};
}


void MinorRadiusOperation::applyOperation() {
	for (int i = 0; i < scene.obstacles.size(); i++) {
		auto& obstacle = scene.obstacles[i];
		if (obstacle.isSelected()) {
			obstacle.descriptor->shape->minorRadius =
				scene.getCurrentNode()->level.obstacleDescriptors[i]->shape->minorRadius + adjustment;
			obstacle.invalidateAllMeshes();
		}
	}
}