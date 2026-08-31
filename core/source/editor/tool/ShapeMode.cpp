#include "editor/tool/ShapeMode.h"

#include "editor/EditorContext.h"
#include "editor/operation/DrawOperation.h"
#include "editor/operation/MinorRadiusOperation.h"
#include "editor/operation/SelectOperation.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/norm.hpp"


void ShapeMode::addGizmos(GizmoRenderer& gizmoRenderer) const {
	if (activeOperation)
		activeOperation->addGizmos(gizmoRenderer);
	else {
		auto addHandles = [&](const EditorObstacle& obstacle, std::optional<bool> pointedCap) {
			auto addHandle = [&](glm::vec2 position, col fillColor, col strokeColor) {
				float radius = std::min(Settings::Sizes.obstacleCapHandleRadius, gizmoRenderer.planarToUIDistance(obstacle.descriptor->shape->minorRadius));
				PanelStyle style = {
					.fillColor = fillColor,
					.strokeColor = strokeColor,
					.cornerRadius = radius,
					.strokeWidth = 2.f,
				};
				gizmoRenderer.addCircle(position, style);
			};

			if (pointedCap) {
				addHandle(*pointedCap ? obstacle.getLeftCapPosition() : obstacle.getRightCapPosition(),
					{Color::White, 0.8f}, {Color::Black, 0.8f});
				addHandle(!*pointedCap ? obstacle.getLeftCapPosition() : obstacle.getRightCapPosition(),
					{Color::White, 0.3f}, {Color::Black, 0.3f});
			} else {
				addHandle(obstacle.getLeftCapPosition(),
					{Color::White, 0.3f}, {Color::Black, 0.3f});
				addHandle(obstacle.getRightCapPosition(),
					{Color::White, 0.3f}, {Color::Black, 0.3f});
			}
		};

		auto pointerPlanarPosition = ctx.camera.screenToPlanarPosition(pointer0Position);

		auto pointedCapInfo = getPointedCapHandleInfo(pointerPlanarPosition);
		auto pointedObstacleIndex = ctx.getPointedObstacleIndex(pointerPlanarPosition, true);
		if (pointedCapInfo)
			addHandles(ctx.scene.obstacles[pointedCapInfo->obstacleIndex], pointedCapInfo->leftCap);
		else if (pointedObstacleIndex)
			addHandles(ctx.scene.obstacles[*pointedObstacleIndex], std::nullopt);

		for (int i = 0; i < ctx.scene.obstacles.size(); i++) {
			const auto& obstacle = ctx.scene.obstacles[i];
			if (obstacle.isSelected() &&
				!(pointedCapInfo && i == pointedCapInfo->obstacleIndex) &&
				!(pointedObstacleIndex && i == *pointedObstacleIndex))
				addHandles(obstacle, std::nullopt);
		}
	}
}


std::optional<Cursor> ShapeMode::queryCursor() const {
	if (activeOperation)
		return activeOperation->queryCursor();

	auto pointerPlanarPosition = ctx.camera.screenToPlanarPosition(pointer0Position);
	if (!getPointedCapHandleInfo(pointerPlanarPosition)) {
		if (auto index = getPointedRimIndex(pointerPlanarPosition)) {
			auto dir = Camera::planarToScreenDirection(ctx.scene.obstacles[*index].getRimProximity(pointerPlanarPosition).direction);
			return Cursor{
				.style = Cursor::Style::DynamicResize,
				.dynamic = true,
				.angle = std::atan2(dir.y, dir.x) + glm::half_pi<float>(),
			};
		}
	}
	return std::nullopt;
}


void ShapeMode::performPrimaryAction(const PointerEvent& upEvent) {
	if (!getPointedCapHandleInfo(ctx.camera.screenToPlanarPosition(pointerDownEvent.position))) {
		auto selectOperation = SelectOperation(
			ctx, TriggerType::Pointer,
			ctx.camera.screenToPlanarPosition(pointerDownEvent.position), true);
		if (selectOperation.start(pointerDownEvent.modifiers)) {
			selectOperation.finish();
			selectOperation.commit();
		}
	}
}


std::unique_ptr<Operation> ShapeMode::startDrag(const PointerEvent& dragStartEvent) {
	auto pointerPlanarPosition = ctx.camera.screenToPlanarPosition(pointerDownEvent.position);

	if (dragStartEvent.button == PointerButton::Primary) {
		if (auto capInfo = getPointedCapHandleInfo(pointerPlanarPosition)) {
			auto& obstacle = ctx.scene.obstacles[capInfo->obstacleIndex];
			auto manipulateCapOperation = std::make_unique<ManipulateCapOperation>(ctx, TriggerType::Pointer, pointerPlanarPosition, capInfo->obstacleIndex, capInfo->leftCap,
				obstacle.getKinematicState()->getAngle() + (capInfo->leftCap ? obstacle.descriptor->shape->getRightCapAngle() : obstacle.descriptor->shape->getLeftCapAngle()));
			if (manipulateCapOperation->start(pointerDownEvent.modifiers))
				return manipulateCapOperation;
		} else if (auto index = getPointedRimIndex(pointerPlanarPosition)) {
			if (!ctx.scene.obstacles[*index].isSelected()) {
				ctx.scene.deselectAll();
				ctx.scene.obstacles[*index].select();
			}
			ctx.scene.selectionFocus = {EntityType::Obstacle, *index};

			auto minorRadiusOperation = std::make_unique<MinorRadiusOperation>(ctx, TriggerType::Pointer, pointerPlanarPosition, minorRadius);
			if (minorRadiusOperation->start(pointerDownEvent.modifiers))
				return minorRadiusOperation;

			ctx.scene.cancelSelectionChange(); // Clean up if minor radius operation fails to start
			return nullptr;
		}

		if (!pointedAtEntity(pointerPlanarPosition)) {
			auto selectOperation = std::make_unique<SelectOperation>(ctx, TriggerType::Pointer, pointerPlanarPosition);
			if (selectOperation->start(pointerDownEvent.modifiers))
				return selectOperation;
		}
	} else if (dragStartEvent.button == PointerButton::Secondary) {
		std::optional<float> tangentAngle = std::nullopt;
		auto sproutingPoint = pointerPlanarPosition;
		if (auto capInfo = getPointedCapHandleInfo(pointerPlanarPosition)) {
			const auto& obstacle = ctx.scene.obstacles[capInfo->obstacleIndex];
			minorRadius = obstacle.descriptor->shape->minorRadius;
			tangentAngle = wrapAngle(obstacle.getKinematicState()->getAngle() + glm::pi<float>() +
				(capInfo->leftCap ? obstacle.descriptor->shape->getLeftCapAngle() : obstacle.descriptor->shape->getRightCapAngle()));
			sproutingPoint = capInfo->leftCap ? obstacle.getLeftCapPosition() : obstacle.getRightCapPosition();
		}
		auto drawOperation = std::make_unique<DrawOperation>(ctx, TriggerType::Pointer, sproutingPoint, minorRadius, tangentAngle);
		if (drawOperation->start(pointerDownEvent.modifiers))
			return drawOperation;
	}

	return nullptr;
}


std::optional<ShapeMode::CapInfo> ShapeMode::getPointedCapHandleInfo(glm::vec2 pointerPlanarPosition) const {
	if (auto index = ctx.getTopObstacleIndex([this, pointerPlanarPosition](const auto& obstacle) {
		float leftCapDistanceSq = length2(pointerPlanarPosition - obstacle.getLeftCapPosition());
		float rightCapDistanceSq = length2(pointerPlanarPosition - obstacle.getRightCapPosition());
		float capHandleRadius = std::min(Settings::Sizes.obstacleCapHandleRadius * ctx.uiToWorldScale, obstacle.descriptor->shape->minorRadius);
		return std::min(leftCapDistanceSq, rightCapDistanceSq) < capHandleRadius * capHandleRadius;
	}, true)) {
		return CapInfo(*index,
			length2(pointerPlanarPosition - ctx.scene.obstacles[*index].getLeftCapPosition())
			< length2(pointerPlanarPosition - ctx.scene.obstacles[*index].getRightCapPosition()));
	}
	return std::nullopt;
}
std::optional<int> ShapeMode::getPointedRimIndex(glm::vec2 pointerPlanarPosition) const {
	if (auto index = ctx.getTopObstacleIndex([this, pointerPlanarPosition](const EditorObstacle& obstacle) {
		return std::abs(obstacle.getRimProximity(pointerPlanarPosition).distance) < Settings::Sizes.obstaclePerimeterHitRadius * ctx.uiToWorldScale;
	})) {
		auto pointedObstacleIndex = ctx.getPointedObstacleIndex(pointerPlanarPosition);
		if (!pointedObstacleIndex || *pointedObstacleIndex == *index)
			return index;
	}
	return std::nullopt;
}