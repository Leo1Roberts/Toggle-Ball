#ifndef EDITOR_H
#define EDITOR_H

#include "Obstacle.h"
#include "Level.h"
#include "Screen.h"

#include <utility>
#include "UIManager.h"


enum class EntityType { None, Ball, Obstacle };
struct EntityReference {
	EntityType type = EntityType::None;
	unsigned short index = 0;

	bool operator==(const EntityReference&) const = default;
};

struct SelectionState {
	EntityReference focus;
	std::vector<EntityReference> selection;
};


struct SelectionUndoNode {
	SelectionUndoNode(SelectionState selectionState, const std::shared_ptr<SelectionUndoNode>& previous = nullptr, const std::shared_ptr<SelectionUndoNode>& next = nullptr) :
		selectionState(std::move(selectionState)), previous(previous), next(next) {}

	SelectionState selectionState;

	std::shared_ptr<SelectionUndoNode> previous;
	std::shared_ptr<SelectionUndoNode> next;
};

struct UndoNode {
	UndoNode(const LevelDescriptor& level, const std::shared_ptr<SelectionUndoNode>& selectionNode, const std::shared_ptr<UndoNode>& previous = nullptr, const std::shared_ptr<UndoNode>& next = nullptr) :
		level(level), selectionNode(selectionNode), previous(previous), next(next) {}

	LevelDescriptor level;
	std::shared_ptr<SelectionUndoNode> selectionNode;

	std::shared_ptr<UndoNode> previous;
	std::shared_ptr<UndoNode> next;
};


class EditorScreen : public Screen {
public:
	explicit EditorScreen(std::unique_ptr<LevelDescriptor> levelToEdit);

	void processEvent(const Event& event) override;
	void update(microseconds dt) override;
	void render() override;

	[[nodiscard]] const LevelDescriptor* getLevel() const { return level.get(); }

private:
	void toggle(bool transition = true);

	void updateObstaclePositions(microseconds dt);

	[[nodiscard]] float getObstacleOpacity(const EditorObstacle& obstacle) const;
	void drawObject(const Mesh<ObjectVertex>* model, const Texture* texture, glm::vec3 position, const glm::mat3& rotation, glm::vec3 scale = glm::vec3(1.f));
	void drawObstacleOutline(const EditorObstacle& obstacle);

	[[nodiscard]] std::shared_ptr<UndoNode> makeUndoNode() const;
	[[nodiscard]] std::shared_ptr<SelectionUndoNode> makeSelectionUndoNode() const;
	void syncLevel();
	void syncSelection();
	void undo();
	void redo();

	std::unique_ptr<LevelDescriptor> level;
	EditorBall ball{};
	std::vector<EditorObstacle> obstacles;

	bool toggled{false};
	Smoother togglePosition{};

	EntityReference selectionFocus;
	std::shared_ptr<UndoNode> currentNode;


	glm::vec3 viewOrigin{0.f};
	float heading = 0.f, pitch = 0.f;
	glm::vec3 viewDirection{};
	float viewDistance{};
	glm::vec3 viewPosition{};

	glm::vec3 viewUpDirection{0.f};
	glm::vec3 viewSunDirection{0.f};

	float halfWidth{}, halfHeight{};

	glm::mat4 worldMatrix{}, viewMatrix{}, projectionMatrix{};
	glm::mat3 viewRotationMatrix{};

	void doResize(int width, int height, float dpiScale) override;

	UIManager uiManager{};
};

#endif // EDITOR_H
