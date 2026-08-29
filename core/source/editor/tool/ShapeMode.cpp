#include "editor/tool/ShapeMode.h"

#include "editor/operation/DrawOperation.h"
#include "editor/operation/MinorRadiusOperation.h"
#include "editor/operation/SelectOperation.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/norm.hpp"


std::optional<Cursor> ShapeMode::queryCursor() const {
	if (activeOperation)
		return activeOperation->queryCursor();

	auto pointerPlanarPosition = camera.screenToPlanarPosition(pointer0Position);
	if (auto index = Operation::getTopObstacleIndex(scene.obstacles, [this, pointerPlanarPosition](const EditorObstacle& obstacle) {
		return std::abs(obstacle.getRimProximity(pointerPlanarPosition).distance) < Settings::Sizes.obstaclePerimeterHitRadius * uiToWorldScale;
	})) {
		auto dir = Camera::planarToScreenDirection(scene.obstacles[*index].getRimProximity(pointerPlanarPosition).direction);
		return Cursor{
			.style = Cursor::Style::DynamicResize,
			.dynamic = true,
			.angle = std::atan2(dir.y, dir.x) + glm::half_pi<float>(),
		};
	}
	return std::nullopt;
}


std::unique_ptr<Operation> ShapeMode::startDrag(const PointerEvent& dragStartEvent) {
	auto pointerPlanarPosition = camera.screenToPlanarPosition(pointerDownEvent.position);

	if (dragStartEvent.button == PointerButton::Primary) {
		if (auto index = Operation::getTopObstacleIndex(scene.obstacles, [this, pointerPlanarPosition](const EditorObstacle& obstacle) {
			return std::abs(obstacle.getRimProximity(pointerPlanarPosition).distance) < Settings::Sizes.obstaclePerimeterHitRadius * uiToWorldScale;
		})) {
			if (!scene.obstacles[*index].isSelected()) {
				scene.deselectAll();
				scene.obstacles[*index].select();
			}
			scene.selectionFocus = {EntityType::Obstacle, *index};

			auto minorRadiusOperation = std::make_unique<MinorRadiusOperation>(scene, camera, TriggerType::Pointer, pointerPlanarPosition, minorRadius);
			if (minorRadiusOperation->start(pointerDownEvent.modifiers))
				return minorRadiusOperation;

			scene.cancelSelectionChange(); // Clean up if minor radius operation fails to start
			return nullptr;
		}

		if (auto entity = pointedAtEntity(pointerDownEvent.position)) {
			if (entity.type == EntityType::Obstacle) {
				auto& obstacle = scene.obstacles[entity.index];
				float leftCapDistanceSq = length2(pointerPlanarPosition - obstacle.getLeftCapPosition());
				float rightCapDistanceSq = length2(pointerPlanarPosition - obstacle.getRightCapPosition());
				if (std::min(leftCapDistanceSq, rightCapDistanceSq) < 0.4f * 0.4f) { // TODO: UI-based distance calculation
					bool manipulateLeftCap = leftCapDistanceSq < rightCapDistanceSq;
					auto manipulateCapOperation = std::make_unique<ManipulateCapOperation>(scene, camera, TriggerType::Pointer, pointerPlanarPosition, obstacle, manipulateLeftCap,
						obstacle.getKinematicState()->getAngle() + (manipulateLeftCap ? obstacle.descriptor->shape->getRightCapAngle() : obstacle.descriptor->shape->getLeftCapAngle()));
					if (manipulateCapOperation->start(pointerDownEvent.modifiers))
						return manipulateCapOperation;
				}
			}
		} else {
			auto selectOperation = std::make_unique<SelectOperation>(scene, camera, TriggerType::Pointer, pointerPlanarPosition);
			if (selectOperation->start(pointerDownEvent.modifiers))
				return selectOperation;
		}
	} else if (dragStartEvent.button == PointerButton::Secondary) {
		auto drawOperation = std::make_unique<DrawOperation>(scene, camera, TriggerType::Pointer, pointerPlanarPosition, minorRadius);
		if (drawOperation->start(pointerDownEvent.modifiers))
			return drawOperation;
	}

	return nullptr;
}