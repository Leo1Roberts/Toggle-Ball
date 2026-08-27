#include "editor/operation/ManipulateCapOperation.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/norm.hpp"


ManipulateCapOperation::ManipulateCapOperation(EditorScene& scene, const Camera& camera, TriggerType trigger, glm::vec2 initialPlanarPosition, EditorObstacle& obstacle, bool leftCap, glm::vec2 tangent)
	: Operation(scene, camera, trigger, initialPlanarPosition), obstacle(obstacle), leftCap(leftCap), tangent(tangent) {
	auto ks = obstacle.getKinematicState();
	auto shape = obstacle.descriptor->shape.get();
	auto capToManipulate = leftCap ? shape->getLeftCap() : shape->getRightCap();
	auto fixedCap = !leftCap ? shape->getLeftCap() : shape->getRightCap();
	auto pos = worldToPlanar(ks->getPosition());
	auto rotation = angleToRotation2D(ks->getAngle());
	initialCapPlanarPosition = pos + rotation * capToManipulate;
	fixedCapPlanarPosition = pos + rotation * fixedCap;
}


OperationResponse ManipulateCapOperation::doProcessEvent(const Event& event) {
	if (auto* pointer = std::get_if<PointerEvent>(&event)) {
		if (pointer->action == PointerAction::Move || pointer->action == PointerAction::Drag) {
			pointerPlanarPosition = camera.screenToPlanarPosition(pointer->position);
			applyOperation();
			return {.consumedEvent = false, .status = OperationStatus::Running};
		}
	}
	return {.consumedEvent = false, .status = OperationStatus::Running};
}


void ManipulateCapOperation::applyOperation() {
	auto capPlanarPosition = initialCapPlanarPosition + pointerPlanarPosition - initialPlanarPosition;
	auto diff = capPlanarPosition - fixedCapPlanarPosition;
	float lengthSq = length2(diff);
	float length = lengthSq > 0.000001f ? std::sqrt(lengthSq) : 0.f;
	obstacle.descriptor->shape = std::make_unique<SegmentSpec>(obstacle.descriptor->shape->minorRadius, 0.f, length);
	auto staticSpec = (StaticSpec*)obstacle.descriptor->motion.get();
	staticSpec->setAngle(std::atan2(diff.y, diff.x));

	obstacle.initKinematicState();
	obstacle.invalidateAllMeshes();
}