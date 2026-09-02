#ifndef EDITOR_CONTEXT_H
#define EDITOR_CONTEXT_H

#include <functional>
#include <optional>
#include <glm/glm.hpp>

struct EntityReference;
class EditorObstacle;
class Camera;
struct EditorQuickSettings;
class EditorScene;


struct EditorQuickSettings {
	bool snap = true;
	struct {
		bool bothStates = false;
		bool individually = false;
	} transform;
	struct {
		bool alignWithTangent = true;
	} shape;
};

enum class SnapType { None, Cap, Spine };
struct SnapResult {
	glm::vec2 value;
	SnapType type = SnapType::None;
	std::optional<float> angle = std::nullopt;
};

struct EditorContext {
	EditorContext(EditorScene& scene, const Camera& camera, const EditorQuickSettings& quickSettings, const float& uiToWorldScale)
		: scene(scene), camera(camera), quickSettings(quickSettings), uiToWorldScale(uiToWorldScale) {}

	[[nodiscard]] std::optional<int> getTopObstacleIndex(const std::function<bool(const EditorObstacle&)>& includePredicate, bool prioritiseSelected = false) const;
	[[nodiscard]] std::optional<int> getPointedObstacleIndex(glm::vec2 pointerPlanarPosition, bool prioritiseSelected = false) const;

	[[nodiscard]] std::vector<int> getPointedObstacleIndices(glm::vec2 pointerPlanarPosition, int excludedObstacleIndex) const;

	[[nodiscard]] SnapResult snapPoint(glm::vec2 point, const EntityReference& excludedEntity) const;
	[[nodiscard]] SnapResult snapPointRestrictedToLine(glm::vec2 point, const EntityReference& excludedEntity, glm::vec2 pointOnLine, float lineAngle) const;

	EditorScene& scene;
	const Camera& camera;
	const EditorQuickSettings& quickSettings;
	const float& uiToWorldScale;
};


#endif // EDITOR_CONTEXT_H
