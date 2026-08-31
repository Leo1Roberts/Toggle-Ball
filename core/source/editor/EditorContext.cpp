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
	float maxHalfDepth = 0.f;
	for (int i = 0; i < scene.obstacles.size(); i++) {
		const auto& obstacle = scene.obstacles[i];
		if (includePredicate(obstacle) &&
			(obstacle.descriptor->shape->getHalfDepth() > maxHalfDepth ||
			(obstacle.descriptor->shape->getHalfDepth() == maxHalfDepth && obstacle.isSelected()))) {
			maxHalfDepth = obstacle.descriptor->shape->getHalfDepth();
			topObstacleIndex = i;
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


SnapResult EditorContext::snapPoint(glm::vec2 point, const EntityReference& excludedEntity) const {
	if (quickSettings.snap) {
		float shortestCapDistanceSq = std::numeric_limits<float>::max();
		glm::vec2 closestCap;
		for (int i = 0; i < scene.obstacles.size(); i++)
			if (!(excludedEntity.type == EntityType::Obstacle && i == excludedEntity.index)) {
				const auto& otherObstacle = scene.obstacles[i];
				float leftCapDistanceSq = length2(point - otherObstacle.getLeftCapPosition());
				float rightCapDistanceSq = length2(point - otherObstacle.getRightCapPosition());
				float minCapDistance = std::min(leftCapDistanceSq, rightCapDistanceSq);
				if (minCapDistance < shortestCapDistanceSq) {
					shortestCapDistanceSq = minCapDistance;
					closestCap = leftCapDistanceSq < rightCapDistanceSq ? otherObstacle.getLeftCapPosition() : otherObstacle.getRightCapPosition();
				}
			}
		float planarSnappingDistance = Settings::Sizes.snappingDistance * uiToWorldScale;
		if (shortestCapDistanceSq < planarSnappingDistance * planarSnappingDistance)
			return {.value = closestCap, .snapped = true};
	}
	return {.value = point, .snapped = false};
}