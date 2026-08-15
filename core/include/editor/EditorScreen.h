#ifndef EDITOR_H
#define EDITOR_H

#include "utilities/Camera.h"
#include "editor/EditorContext.h"
#include "editor/EditorScene.h"
#include "editor/GizmoRenderer.h"
#include "editor/operation/Operation.h"
#include "Screen.h"
#include "editor/tool/ToolMode.h"
#include "ui/UIManager.h"


class UITextBubble;
class UIVerticalList;

class EditorScreen : public Screen {
public:
	explicit EditorScreen(std::unique_ptr<LevelDescriptor> levelToEdit);

	void processEvent(const Event& event) override;
	void update(microseconds dt) override;
	void render() override;

	[[nodiscard]] const LevelDescriptor* getLevel() const { return scene.level.get(); }

private:
	void doResize() override;

	void drawObject(const Mesh<ObjectVertex>* model, const Texture* texture, glm::vec3 position, const glm::mat3& rotation, glm::vec3 scale = glm::vec3(1.f)) const;
	void drawObstacleOutline(const EditorObstacle& obstacle) const;
	[[nodiscard]] float getObstacleOpacity(const EditorObstacle& obstacle) const;

	bool panning = false;
	float uiToWorldScale{};
	void updateEphemeralMeshes();
	void updateObstacleMotionPropertiesList();

	glm::vec2 pointer0Position{};

	EditorQuickSettings quickSettings;
	EditorScene scene;
	Camera camera;
	UIManager uiManager;
	GizmoRenderer gizmoRenderer{&uiManager, &camera};
	EditorContext context;

	SelectionState cachedSelectionState{};

	UINode* viewportUI;

	UITextBubble* stateIndicator;
	void updateStateIndicator();
	UIVerticalList* obstacleMotionPropertiesList;

	std::unique_ptr<ToolMode> currentToolMode;
	std::unique_ptr<Operation> activeOperation;
	Operation* startOperation(std::unique_ptr<Operation> operation);
	void finishOperation();
};

#endif // EDITOR_H
