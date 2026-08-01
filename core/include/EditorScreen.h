#ifndef EDITOR_H
#define EDITOR_H

#include "Camera.h"
#include "EditorScene.h"
#include "GizmoRenderer.h"
#include "Obstacle.h"
#include "Operation.h"
#include "Level.h"
#include "Screen.h"
#include "ToolMode.h"
#include "UIManager.h"


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

	glm::vec2 mainPointerPosition{};

	EditorQuickSettings quickSettings;
	EditorScene scene;
	Camera camera;
	UIManager uiManager;
	GizmoRenderer gizmoRenderer{&uiManager, &camera};
	EditorContext context;

	std::unique_ptr<ToolMode> currentToolMode;
	std::unique_ptr<Operation> activeOperation;
};

#endif // EDITOR_H
