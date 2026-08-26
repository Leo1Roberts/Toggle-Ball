#ifndef TOOL_MODE_H
#define TOOL_MODE_H

#include "editor/operation/Operation.h"
#include "editor/operation/TransformOperation.h"
#include "io/Event.h"


class UINode;
class Camera;
class EditorScene;
class GizmoRenderer;


struct ToolModeResponse {
	bool consumedEvent = false;
	bool operationChanged = false;
};

class ToolMode : public ICursorProvider {
public:
	~ToolMode() override = default;

	[[nodiscard]] ToolModeResponse processEvent(const Event& event);
	[[nodiscard]] ToolModeResponse processObstacleExistenceAction(ActionCode actionCode, byte modifiers, const TransformQuickSettings& settings = {});

	[[nodiscard]] virtual std::vector<BindingHint> getBindingHints() const { return {}; }
	virtual void populateToolbar(UINode& toolbar) {}
	void createOperationUI(UINode& container) const;
	virtual void renderGizmos(GizmoRenderer& gizmoRenderer) {}

	void cancelActiveOperation();
	void commitActiveOperation();

	[[nodiscard]] bool hasActiveOperation() const { return activeOperation != nullptr; }

	std::optional<Cursor> queryCursor() const override;

protected:
	explicit ToolMode(EditorScene& scene, const Camera& camera)
		: scene(scene), camera(camera) {}

	EditorScene& scene;
	const Camera& camera;

	std::unique_ptr<Operation> activeOperation;

	PointerEvent pointerDownEvent;
	glm::vec2 pointer0Position{};

	[[nodiscard]] bool pointedAtBall(glm::vec2 pointerPosition) const;
	[[nodiscard]] bool pointedAtObstacle(glm::vec2 pointerPosition) const;
	[[nodiscard]] bool pointedAtEntity(glm::vec2 pointerPosition) const {
		return pointedAtBall(pointerPosition) || pointedAtObstacle(pointerPosition);
	}

private:
	[[nodiscard]] virtual ToolModeResponse doProcessEvent(const Event& event) { return {}; }

	// Pointer down and up on the same spot
	virtual void performPrimaryAction(const PointerEvent& upEvent);
	virtual void performSecondaryAction(const PointerEvent& upEvent) {}
	[[nodiscard]] virtual std::unique_ptr<Operation> startDrag(const PointerEvent& dragStartEvent) { return nullptr; }

	bool dragging = false;
	bool pointerPrimaryDown = false;
	bool pointerSecondaryDown = false;
};

#endif // TOOL_MODE_H
