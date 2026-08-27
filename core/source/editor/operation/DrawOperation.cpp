#include "editor/operation/DrawOperation.h"

#include "editor/operation/ManipulateCapOperation.h"


DrawOperation::DrawOperation(EditorScene& scene, const Camera& camera, TriggerType trigger, glm::vec2 initialPlanarPosition, float minorRadius, glm::vec2 tangent)
	: Operation(scene, camera, trigger, initialPlanarPosition) {
	scene.deselectAll();
	auto shape = std::make_unique<SegmentSpec>(minorRadius, 0.f, 0.f);
	auto motion = std::make_unique<StaticSpec>(initialPlanarPosition, 0.f);
	auto descriptor = scene.level->obstacleDescriptors.emplace_back(
		std::make_unique<ObstacleDescriptor>(std::move(shape), std::move(motion))).get();
	auto& obstacle = scene.obstacles.emplace_back(descriptor);
	scene.selectionFocus = {EntityType::Obstacle, (int)scene.obstacles.size() - 1};
	manipulateCapOperation = std::make_unique<ManipulateCapOperation>(scene, camera, trigger, initialPlanarPosition, obstacle, false);
}