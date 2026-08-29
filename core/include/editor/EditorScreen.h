#ifndef EDITOR_H
#define EDITOR_H

#include "utilities/Camera.h"
#include "editor/EditorScene.h"
#include "editor/GizmoRenderer.h"
#include "editor/operation/Operation.h"
#include "Screen.h"
#include "editor/tool/ToolMode.h"
#include "tool/ShapeMode.h"
#include "tool/TransformMode.h"


class UIHorizontalList;
class UIVerticalList;

class EditorScreen : public Screen {
public:
	explicit EditorScreen(std::unique_ptr<LevelDescriptor> levelToEdit, const std::function<void()>& testLevelCallback);

	void processEvent(const Event& event) override;
	void update(microseconds dt) override;
	void render() override;

	[[nodiscard]] const LevelDescriptor* getLevel() const { return scene.level.get(); }

	[[nodiscard]] std::optional<Cursor> queryCursor() const override;

private:
	void doResize() override;

	void drawObject(const Mesh<ObjectVertex>* model, const Texture* texture, glm::vec3 position, const glm::mat3& rotation, glm::vec3 scale = glm::vec3(1.f)) const;
	void drawObstacleOutline(EditorObstacle& obstacle) const;
	[[nodiscard]] float getObstacleOpacity(const EditorObstacle& obstacle) const;

	bool panning = false;
	float uiToWorldScale{};
	void updateEphemeralMeshes();
	void updateObstacleMotionPropertiesList();

	EditorScene scene;
	Camera camera;
	GizmoRenderer gizmoRenderer{uiManager, camera};

	SelectionState cachedSelectionState{};

	UIContainer* viewportUI;

	TransformMode transformMode;
	ShapeMode shapeMode;
	ToolMode* currentMode = nullptr;
	void selectMode(ToolMode* mode);

	void updateDynamicUI();
	UIContainer* operationUI;
	UIHorizontalList* bindingHints;
	void updateToolbar();
	UIContainer* toolbar;

	bool obstacleMotionPropertiesListValid = false;
	UIVerticalList* obstacleMotionPropertiesList;
};

#endif // EDITOR_H
