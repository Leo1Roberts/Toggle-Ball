#include "editor/operation/ManipulateCapsOperation.h"


ManipulateCapsOperation::ManipulateCapsOperation(const EditorContext& ctx, TriggerType trigger, glm::vec2 initialPlanarPosition, const std::vector<CapInfo>& allCapsInfo) :
	Operation(ctx, trigger, initialPlanarPosition) {
	std::vector<EntityReference> manipulatedEntities(allCapsInfo.size());
	for (auto info : allCapsInfo)
		manipulatedEntities.emplace_back(EntityType::Obstacle, info.obstacleIndex);

	manipulateCapOperations.reserve(allCapsInfo.size());

	for (auto& info : allCapsInfo) {
		const auto& obstacle = ctx.scene.obstacles[info.obstacleIndex];
		manipulateCapOperations.emplace_back(ctx, trigger, initialPlanarPosition, info.obstacleIndex, info.leftCap,
			obstacle.getKinematicState()->getAngle() + obstacle.descriptor->shape->getCapAngle(!info.leftCap),
			manipulatedEntities);
	}
}


void ManipulateCapsOperation::addGizmos(GizmoRenderer& gizmoRenderer) const {
	for (const auto& capOperation : manipulateCapOperations)
		capOperation.addGizmos(gizmoRenderer);
}


OperationResponse ManipulateCapsOperation::doProcessEvent(const Event& event) {
	OperationResponse response;
	for (auto& capOperation : manipulateCapOperations)
		response = capOperation.doProcessEvent(event);
	return response;
}


void ManipulateCapsOperation::applyOperation() {
	for (auto& capOperation : manipulateCapOperations)
		capOperation.applyOperation();
}


void ManipulateCapsOperation::applyModifiers(byte mods) {
	for (auto& capOperation : manipulateCapOperations)
		capOperation.applyModifiers(mods);
}