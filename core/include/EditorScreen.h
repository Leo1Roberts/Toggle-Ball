#ifndef EDITOR_H
#define EDITOR_H

#include "Camera.h"
#include "EditorScene.h"
#include "Obstacle.h"
#include "Level.h"
#include "Screen.h"

#include "UIManager.h"


class EditorScreen : public Screen {
public:
	explicit EditorScreen(std::unique_ptr<LevelDescriptor> levelToEdit);

	void processEvent(const Event& event) override;
	void update(microseconds dt) override;
	void render() override;

	[[nodiscard]] const LevelDescriptor* getLevel() const { return scene.getLevel(); }

private:
	void doResize() override;

	void drawObject(const Mesh<ObjectVertex>* model, const Texture* texture, glm::vec3 position, const glm::mat3& rotation, glm::vec3 scale = glm::vec3(1.f)) const;
	void drawObstacleOutline(const EditorObstacle& obstacle) const;
	[[nodiscard]] float getObstacleOpacity(const EditorObstacle& obstacle) const;

	void updateView();

	EditorScene scene;
	Camera camera;
	UIManager uiManager;

	float centreDotRadius{};
};

#endif // EDITOR_H
