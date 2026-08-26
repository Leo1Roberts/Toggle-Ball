#include "editor/operation/DrawOperation.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/norm.hpp"


DrawOperation::DrawOperation(EditorScene& scene, const Camera& camera, TriggerType trigger, glm::vec2 initialPlanarPosition, float minorRadius, glm::vec2 tangent)
	: Operation(scene, camera, trigger, initialPlanarPosition), minorRadius(minorRadius), tangent(tangent), terminalPlanarPosition(initialPlanarPosition) {
	scene.deselectAll();
	auto shape = std::make_unique<SegmentSpec>(minorRadius, 0.f, 0.f);
	auto motion = std::make_unique<StaticSpec>(initialPlanarPosition, 0.f); // Later code assumes this is a StaticSpec
	auto descriptor = scene.level->obstacleDescriptors.emplace_back(
		std::make_unique<ObstacleDescriptor>(std::move(shape), std::move(motion))).get();
	obstacle = &scene.obstacles.emplace_back(descriptor);
	scene.selectionFocus = {EntityType::Obstacle, (int)scene.obstacles.size() - 1};
}


OperationResponse DrawOperation::doProcessEvent(const Event& event) {
	if (auto* pointer = std::get_if<PointerEvent>(&event)) {
		if (pointer->action == PointerAction::Move || pointer->action == PointerAction::Drag) {
			terminalPlanarPosition = camera.screenToPlanarPosition(pointer->position);
			applyOperation();
			return {.consumedEvent = false, .status = OperationStatus::Running};
		}
	}
	return {.consumedEvent = false, .status = OperationStatus::Running};
}


void DrawOperation::applyOperation() {
	auto diff = terminalPlanarPosition - initialPlanarPosition;
	float lengthSq = length2(diff);
	float length = lengthSq ? std::sqrt(lengthSq) : 0.f;
	obstacle->descriptor->shape = std::make_unique<SegmentSpec>(minorRadius, 0.f, length);
	auto staticSpec = (StaticSpec*)obstacle->descriptor->motion.get();
	staticSpec->setAngle(std::atan2(diff.y, diff.x));

	obstacle->initKinematicState();
	obstacle->invalidateAllMeshes();
}