#include "editor/tool/ShapeMode.h"

#include "editor/EditorContext.h"
#include "editor/operation/DrawOperation.h"
#include "editor/operation/MinorRadiusOperation.h"
#include "editor/operation/SelectOperation.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "editor/operation/ManipulateMidsectionOperation.h"
#include "glm/gtx/norm.hpp"


void ShapeMode::addGizmos(GizmoRenderer& gizmoRenderer) const {
	if (activeOperation)
		activeOperation->addGizmos(gizmoRenderer);
	else {
		auto pointerPlanarPosition = ctx.camera.screenToPlanarPosition(pointer0Position);

		auto addHandles = [&](const EditorObstacle& obstacle, std::optional<bool> pointedCap) {
			auto addHandle = [&](glm::vec2 position, col fillColor, col strokeColor) {
				float radius = std::min(Settings::Sizes.obstacleHandleRadius, gizmoRenderer.planarToUIDistance(obstacle.descriptor->shape->minorRadius));
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

		auto pointedCapInfo = getPointedCapHandleInfo(pointerPlanarPosition);
		auto pointedObstacleIndex = ctx.getPointedObstacleIndex(pointerPlanarPosition, true);
		if (pointedCapInfo)
			addHandles(ctx.scene.obstacles[pointedCapInfo->obstacleIndex], pointedCapInfo->leftCap);
		else if (pointedObstacleIndex) {
			const auto& obstacle = ctx.scene.obstacles[*pointedObstacleIndex];

			addHandles(obstacle, std::nullopt);

			if (auto info = getMidsectionHandleInfo(obstacle, pointerPlanarPosition)) {
				float radius = std::min(Settings::Sizes.obstacleHandleRadius, gizmoRenderer.planarToUIDistance(obstacle.descriptor->shape->minorRadius));
				float opacity = info->pointed ? 0.8f : 0.3f;
				gizmoRenderer.addSplitCircle(info->position, radius, info->angle, {
					.fillColor = {Color::White, opacity},
					.strokeColor = {Color::Black, opacity},
					.cornerRadius = 0.f,
					.strokeWidth = 2.f,
				}, {
					.fillColor = {Color::White, opacity},
					.strokeColor = {Color::Black, opacity},
					.cornerRadius = radius,
					.strokeWidth = 2.f,
				});
			}
		}

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
	if (!getPointedCapHandleInfo(ctx.camera.screenToPlanarPosition(pointerDownEvent.position)))
		ToolMode::performPrimaryAction(upEvent);
}


std::unique_ptr<Operation> ShapeMode::startDrag(const PointerEvent& dragStartEvent) {
	auto pointerPlanarPosition = ctx.camera.screenToPlanarPosition(pointerDownEvent.position);

	if (dragStartEvent.button == PointerButton::Primary) {
		if (auto capInfo = getPointedCapHandleInfo(pointerPlanarPosition)) {
			auto& obstacle = ctx.scene.obstacles[capInfo->obstacleIndex];
			ctx.scene.deselectAll();
			obstacle.select();
			ctx.scene.selectionFocus = {EntityType::Obstacle, capInfo->obstacleIndex};

			auto manipulateCapOperation = std::make_unique<ManipulateCapOperation>(ctx, TriggerType::Pointer, pointerPlanarPosition, capInfo->obstacleIndex, capInfo->leftCap,
				obstacle.getKinematicState()->getAngle() + (capInfo->leftCap ? obstacle.descriptor->shape->getRightCapAngle() : obstacle.descriptor->shape->getLeftCapAngle()));
			if (manipulateCapOperation->start(pointerDownEvent.modifiers))
				return manipulateCapOperation;

			ctx.scene.cancelSelectionChange();
			return nullptr;
		}

		if (auto index = getPointedRimIndex(pointerPlanarPosition)) {
			auto& obstacle = ctx.scene.obstacles[*index];
			if (!obstacle.isSelected()) {
				ctx.scene.deselectAll();
				obstacle.select();
			}
			ctx.scene.selectionFocus = {EntityType::Obstacle, *index};

			auto minorRadiusOperation = std::make_unique<MinorRadiusOperation>(ctx, TriggerType::Pointer, pointerPlanarPosition, minorRadius);
			if (minorRadiusOperation->start(pointerDownEvent.modifiers))
				return minorRadiusOperation;

			ctx.scene.cancelSelectionChange();
			return nullptr;
		}

		if (auto index = ctx.getPointedObstacleIndex(pointerPlanarPosition)) {
			auto& obstacle = ctx.scene.obstacles[*index];
			if (auto info = getMidsectionHandleInfo(obstacle, pointerPlanarPosition)) {
				if (info->pointed) {
					ctx.scene.deselectAll();
					obstacle.select();
					ctx.scene.selectionFocus = {EntityType::Obstacle, *index};

					auto curvatureOperation = std::make_unique<ManipulateMidsectionOperation>(ctx, TriggerType::Pointer, pointerPlanarPosition, *index, info->position);
					if (curvatureOperation->start(pointerDownEvent.modifiers))
						return curvatureOperation;

					ctx.scene.cancelSelectionChange();
					return nullptr;
				}
			}
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
		} else if (auto index = ctx.getPointedObstacleIndex(pointerPlanarPosition, true)) {
			const auto& obstacle = ctx.scene.obstacles[*index];
			if (auto info = getMidsectionHandleInfo(obstacle, pointerPlanarPosition)) {
				if (info->pointed) {
					minorRadius = obstacle.descriptor->shape->minorRadius;
					tangentAngle = wrapAngle(info->angle + glm::pi<float>());
					sproutingPoint = info->position;
				}
			}
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
		float capHandleRadius = std::min(Settings::Sizes.obstacleHandleRadius * ctx.uiToWorldScale, obstacle.descriptor->shape->minorRadius);
		return std::min(leftCapDistanceSq, rightCapDistanceSq) < capHandleRadius * capHandleRadius;
	}, true)) {
		return CapInfo(*index,
			length2(pointerPlanarPosition - ctx.scene.obstacles[*index].getLeftCapPosition())
			< length2(pointerPlanarPosition - ctx.scene.obstacles[*index].getRightCapPosition()));
	}
	return std::nullopt;
}

std::optional<ShapeMode::MidsectionHandleInfo> ShapeMode::getMidsectionHandleInfo(const EditorObstacle& obstacle, glm::vec2 pointerPlanarPosition) const {
	auto proximityInfo = obstacle.getSpineProximity(pointerPlanarPosition);
	auto offset = length2(proximityInfo.direction) < 0.00000001f
		? glm::vec2(0.f)
		: normalize(proximityInfo.direction) * proximityInfo.distance;
	auto handlePosition = pointerPlanarPosition - offset;

	if (length2(obstacle.getLeftCapPosition() - handlePosition) > 0.00000001f &&
		length2(obstacle.getRightCapPosition() - handlePosition) > 0.00000001f) {
		float radius = std::min(Settings::Sizes.obstacleHandleRadius * ctx.uiToWorldScale, obstacle.descriptor->shape->minorRadius);
		return MidsectionHandleInfo(length2(pointerPlanarPosition - handlePosition) <= radius * radius,
			handlePosition, std::atan2(offset.y, offset.x));
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