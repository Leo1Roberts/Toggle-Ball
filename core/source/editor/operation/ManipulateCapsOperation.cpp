#include "editor/operation/ManipulateCapsOperation.h"


ManipulateCapsOperation::ManipulateCapsOperation(const EditorContext& ctx, TriggerType trigger, glm::vec2 initialPlanarPosition, const std::vector<CapInfo>& allCapsInfo) :
	Operation(ctx, trigger, initialPlanarPosition) {
	std::vector<EntityReference> manipulatedEntities;
	manipulatedEntities.reserve(allCapsInfo.size());
	for (auto info : allCapsInfo)
		manipulatedEntities.emplace_back(EntityType::Obstacle, info.obstacleIndex);

	manipulateCapOperations.reserve(allCapsInfo.size());

	for (auto& info : allCapsInfo) {
		const auto& obstacle = ctx.scene.obstacles[info.obstacleIndex];
		manipulateCapOperations.emplace_back(ctx, trigger, initialPlanarPosition, info.obstacleIndex, info.leftCap,
			obstacle.getKinematicState()->getAngle() + obstacle.descriptor->shape->getCapAngle(!info.leftCap),
			manipulatedEntities, true);
	}
}


void ManipulateCapsOperation::addGizmos(GizmoRenderer& gizmoRenderer) const {
	for (const auto& capOperation : manipulateCapOperations)
		capOperation.addGizmos(gizmoRenderer);
}


OperationResponse ManipulateCapsOperation::doProcessEvent(const Event& event) {
	if (auto* pointer = std::get_if<PointerEvent>(&event)) {
		if (pointer->action == PointerAction::Move || pointer->action == PointerAction::Drag) {
			pointerPlanarPosition = ctx.camera.screenToPlanarPosition(pointer->position);
			for (auto& capOperation : manipulateCapOperations)
				capOperation.pointerPlanarPosition = pointerPlanarPosition;
			applyOperation();
			return {.consumedEvent = false, .status = OperationStatus::Running};
		}
	}

	return {.consumedEvent = false, .status = OperationStatus::Running};
}


void ManipulateCapsOperation::applyOperation() {
	std::vector<ManipulateCapOperation::Restriction::Line> lineRestrictions;
	auto idealHandlePosition = manipulateCapOperations[0].initialCapPlanarPosition + pointerPlanarPosition - initialPlanarPosition;

	for (auto& capOperation : manipulateCapOperations) {
		SnapResult idealHandle = {
			.value = idealHandlePosition,
			.type = SnapType::Cap,
			.angle = wrapAngle(capOperation.initialAngle + capOperation.initialDescriptor.shape->getCapAngle(capOperation.leftCap) + glm::pi<float>()),
		};
		auto restriction = capOperation.getRestriction(idealHandle);
		if (restriction.impossible)
			return;
		if (restriction.line) {
			if (lineRestrictions.size() == 2)
				return;
			lineRestrictions.push_back(*restriction.line);
		}
	}

	if (lineRestrictions.empty())
		for (auto& capOperation : manipulateCapOperations) {
			SnapResult handle = {
				.value = idealHandlePosition,
				.type = SnapType::Cap,
				.angle = wrapAngle(capOperation.initialAngle + capOperation.initialDescriptor.shape->getCapAngle(capOperation.leftCap) + glm::pi<float>()),
			};
			capOperation.applyOperationWithSnapResult(handle, true);
		}
	else {
		glm::vec2 handlePosition;

		if (lineRestrictions.size() == 1) {
			glm::vec2 dir = {std::cos(lineRestrictions[0].angle), std::sin(lineRestrictions[0].angle)};
			handlePosition = lineRestrictions[0].point + dir * dot(dir, idealHandlePosition - lineRestrictions[0].point);
		} else {
			glm::vec2 d1(std::cos(lineRestrictions[0].angle), std::sin(lineRestrictions[0].angle));
			glm::vec2 d2(std::cos(lineRestrictions[1].angle), std::sin(lineRestrictions[1].angle));

			float det = d1.x * d2.y - d1.y * d2.x;

			if (std::abs(det) < 0.0001f)
				return; // (nearly) parallel

			glm::vec2 dp = lineRestrictions[1].point - lineRestrictions[0].point;
			float t = (dp.x * d2.y - dp.y * d2.x) / det;

			handlePosition = lineRestrictions[0].point + t * d1;
		}

		int lineRestrictionCount = 0;
		for (auto& capOperation : manipulateCapOperations) {
			SnapResult handle = {
				.value = handlePosition,
				.type = SnapType::Cap,
				.angle = wrapAngle(capOperation.initialAngle + capOperation.initialDescriptor.shape->getCapAngle(capOperation.leftCap) + glm::pi<float>()),
			};
			auto restriction = capOperation.getRestriction(handle);
			if (restriction.impossible)
				return;
			if (restriction.line) {
				lineRestrictionCount++;
				if (lineRestrictionCount > lineRestrictions.size())
					return;
			}
		}
		for (auto& capOperation : manipulateCapOperations) {
			SnapResult handle = {
				.value = handlePosition,
				.type = SnapType::Cap,
				.angle = wrapAngle(capOperation.initialAngle + capOperation.initialDescriptor.shape->getCapAngle(capOperation.leftCap) + glm::pi<float>()),
			};
			capOperation.applyOperationWithSnapResult(handle, true);
		}
	}
}


void ManipulateCapsOperation::applyModifiers(byte mods) {
	for (auto& capOperation : manipulateCapOperations)
		capOperation.useSnappedTangent = mods & MOD_ALT;
}