#include "editor/EditorContext.h"

#include "editor/EditorScene.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/norm.hpp"


std::optional<int> EditorContext::getTopObstacleIndex(const std::function<bool(const EditorObstacle&)>& includePredicate, bool prioritiseSelected) const {
	if (prioritiseSelected) {
		int topSelectedObstacleIndex = -1;
		float maxSelectedHalfDepth = 0.f;

		for (int i = 0; i < scene.obstacles.size(); i++) {
			const auto& obstacle = scene.obstacles[i];
			if (obstacle.isSelected() && includePredicate(obstacle) &&
				obstacle.descriptor->shape->getHalfDepth() >= maxSelectedHalfDepth) {
				maxSelectedHalfDepth = obstacle.descriptor->shape->getHalfDepth();
				topSelectedObstacleIndex = i;
			}
		}

		if (topSelectedObstacleIndex >= 0)
			return topSelectedObstacleIndex;
	}

	int topObstacleIndex = -1;
	bool topIsSelected = false;
	float maxHalfDepth = 0.f;
	for (int i = 0; i < scene.obstacles.size(); i++) {
		const auto& obstacle = scene.obstacles[i];
		if (includePredicate(obstacle)) {
			if (includePredicate(obstacle) && (
				obstacle.descriptor->shape->getHalfDepth() > maxHalfDepth ||
				obstacle.descriptor->shape->getHalfDepth() == maxHalfDepth && obstacle.isSelected() >= topIsSelected)) {
				maxHalfDepth = obstacle.descriptor->shape->getHalfDepth();
				topObstacleIndex = i;
				topIsSelected = obstacle.isSelected();
			}
		}
	}

	if (topObstacleIndex >= 0)
		return topObstacleIndex;

	return std::nullopt;
}
std::optional<int> EditorContext::getPointedObstacleIndex(glm::vec2 pointerPlanarPosition, bool prioritiseSelected) const {
	return getTopObstacleIndex([pointerPlanarPosition](const EditorObstacle& obstacle) {
		return obstacle.isInSelectBox(SelectBox(pointerPlanarPosition));
	}, prioritiseSelected);
}

std::vector<int> EditorContext::getPointedObstacleIndices(glm::vec2 pointerPlanarPosition, int excludedObstacleIndex) const {
	std::vector<int> indices;
	auto hitTestBox = SelectBox(pointerPlanarPosition);
	for (int i = 0; i < scene.obstacles.size(); i++)
		if (i != excludedObstacleIndex && scene.obstacles[i].isInSelectBox(hitTestBox))
			indices.push_back(i);

	return indices;
}


SnapResult EditorContext::snapPoint(const glm::vec2 point, const EntityReference& excludedEntity) const {
	if (quickSettings.snap) {
		float shortestCapDistanceSq = std::numeric_limits<float>::max();
		glm::vec2 closestCap;
		float closestCapTangentAngle;
		float shortestSpineDistance = std::numeric_limits<float>::max();
		glm::vec2 closestSpinePoint;
		glm::vec2 closestSpineNormal;

		for (int i = 0; i < scene.obstacles.size(); i++)
			if (!(excludedEntity.type == EntityType::Obstacle && i == excludedEntity.index)) {
				const auto& otherObstacle = scene.obstacles[i];
				float leftCapDistanceSq = length2(point - otherObstacle.getLeftCapPosition());
				float rightCapDistanceSq = length2(point - otherObstacle.getRightCapPosition());
				float minCapDistance = std::min(leftCapDistanceSq, rightCapDistanceSq);
				if (minCapDistance < shortestCapDistanceSq) {
					shortestCapDistanceSq = minCapDistance;
					closestCap = leftCapDistanceSq < rightCapDistanceSq ? otherObstacle.getLeftCapPosition() : otherObstacle.getRightCapPosition();
					closestCapTangentAngle = otherObstacle.getKinematicState()->getAngle() +
						(leftCapDistanceSq < rightCapDistanceSq
						? otherObstacle.descriptor->shape->getLeftCapAngle()
						: otherObstacle.descriptor->shape->getRightCapAngle());
				}
				auto proximityInfo = otherObstacle.getSpineProximity(point);
				if (proximityInfo.distance < shortestSpineDistance) {
					shortestSpineDistance = proximityInfo.distance;
					glm::vec2 offset = length2(proximityInfo.direction) < 0.00000001f
						? glm::vec2(0.f)
						: normalize(proximityInfo.direction) * proximityInfo.distance;
					closestSpinePoint = point - offset;
					closestSpineNormal = offset;
				}
			}

		float planarSnappingDistance = Settings::Sizes.snappingDistance * uiToWorldScale;
		float planarSnappingDistanceSq = planarSnappingDistance * planarSnappingDistance;
		if (shortestCapDistanceSq < planarSnappingDistanceSq)
			return {.value = closestCap, .type = SnapType::Cap, .angle = closestCapTangentAngle};
		if (shortestSpineDistance < planarSnappingDistance)
			return {
				.value = closestSpinePoint, .type = SnapType::Spine,
				.angle = std::atan2(closestSpineNormal.y, closestSpineNormal.x)
			};
	}
	return {.value = point};
}

SnapResult EditorContext::snapPointRestrictedToShape(glm::vec2 point, const EntityReference& excludedEntity, const std::function<std::vector<glm::vec2>(const EditorObstacle&)>& getPointsOnShape) const {
	if (quickSettings.snap) {
		float shortestSpineDistanceSq = std::numeric_limits<float>::max();
		glm::vec2 closestSpinePoint;
		int closestSpineIndex = -1;

		for (int i = 0; i < scene.obstacles.size(); i++)
			if (!(excludedEntity.type == EntityType::Obstacle && i == excludedEntity.index)) {
				const auto& otherObstacle = scene.obstacles[i];
				for (auto intersection : getPointsOnShape(otherObstacle)) {
					float distanceSq = length2(intersection - point);
					if (distanceSq < shortestSpineDistanceSq) {
						shortestSpineDistanceSq = distanceSq;
						closestSpinePoint = intersection;
						closestSpineIndex = i;
					}
				}
			}

		if (closestSpineIndex >= 0) {
			float planarSnappingDistance = Settings::Sizes.snappingDistance * uiToWorldScale;
			float planarSnappingDistanceSq = planarSnappingDistance * planarSnappingDistance;
			if (shortestSpineDistanceSq < planarSnappingDistanceSq) {
				const auto& target = scene.obstacles[closestSpineIndex];
				float leftCapDistanceSq = length2(point - target.getLeftCapPosition());
				float rightCapDistanceSq = length2(point - target.getRightCapPosition());
				if (std::min(leftCapDistanceSq, rightCapDistanceSq) < 0.00000001f)
					return {.value = leftCapDistanceSq < rightCapDistanceSq
						? target.getLeftCapPosition()
						: target.getRightCapPosition(),
						.type = SnapType::Cap};

				return { .value = closestSpinePoint, .type = SnapType::Spine };
			}
		}
	}
	return {.value = point};
}
SnapResult EditorContext::snapPointRestrictedToLine(glm::vec2 point, const EntityReference& excludedEntity, glm::vec2 pointOnLine, float lineAngle) const {
	return snapPointRestrictedToShape(point, excludedEntity,
		[=](const EditorObstacle& obstacle) { return obstacle.getPointsOnLine(pointOnLine, lineAngle); });
}
SnapResult EditorContext::snapPointRestrictedToCircle(glm::vec2 point, const EntityReference& excludedEntity, glm::vec2 circleCentre, float circleRadius) const {
	return snapPointRestrictedToShape(point, excludedEntity,
		[=](const EditorObstacle& obstacle) { return obstacle.getPointsOnCircle(circleCentre, circleRadius); });
}