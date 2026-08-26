#include "editor/operation/MinorRadiusOperation.h"


OperationResponse MinorRadiusOperation::doProcessEvent(const Event& event) {
	if (auto* pointer = std::get_if<PointerEvent>(&event)) {
		if (pointer->action == PointerAction::Move || pointer->action == PointerAction::Drag) {
			glm::vec2 pointerPlanarPosition = camera->screenToPlanarPosition(pointer->position);
			adjustment = scene->getCurrentNode()->level.obstacleDescriptors[scene->selectionFocus.index]->shape->getRimProximity(*scene->obstacles[scene->selectionFocus.index].getKinematicState(), pointerPlanarPosition).distance - initialDistance;
			applyOperation();
			return {.consumedEvent = false, .status = OperationStatus::Running};
		}
	}
	return {.consumedEvent = false, .status = OperationStatus::Running};
}


void MinorRadiusOperation::applyOperation() {
	for (int i = 0; i < scene->obstacles.size(); i++) {
		auto& obstacle = scene->obstacles[i];
		if (obstacle.isSelected()) {
			obstacle.descriptor->shape->minorRadius =
				scene->getCurrentNode()->level.obstacleDescriptors[i]->shape->minorRadius + adjustment;

			obstacle.invalidateAllMeshes();
		}
	}
}