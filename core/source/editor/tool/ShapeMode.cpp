#include "editor/tool/ShapeMode.h"

#include "editor/operation/DrawOperation.h"
#include "editor/operation/MinorRadiusOperation.h"
#include "editor/operation/SelectOperation.h"


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

		if (!pointedAtEntity(pointerDownEvent.position)) {
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