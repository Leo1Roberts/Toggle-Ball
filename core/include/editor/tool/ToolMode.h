#ifndef TOOL_MODE_H
#define TOOL_MODE_H

#include "editor/operation/ManipulateCapsOperation.h"
#include "editor/operation/ManipulateMidsectionOperation.h"
#include "editor/operation/Operation.h"
#include "io/Event.h"


class UINode;
class Camera;
class EditorScene;
class GizmoRenderer;


struct ToolModeResponse {
	bool consumedEvent = false;
	bool operationChanged = false;
};

class ToolMode : public IGizmoProvider, public ICursorProvider {
public:
	~ToolMode() override = default;

	[[nodiscard]] ToolModeResponse processEvent(const Event& event);
	[[nodiscard]] ToolModeResponse processObstacleExistenceAction(ActionCode actionCode, byte modifiers);

	[[nodiscard]] virtual std::vector<BindingHint> getBindingHints() const;
	void createOperationUI(UINode& container) const;

	void cancelActiveOperation();
	void commitActiveOperation();

	[[nodiscard]] bool hasActiveOperation() const { return activeOperation != nullptr; }
	[[nodiscard]] bool shapeTypeMayHaveChanged() const {
		return
		dynamic_cast<ManipulateMidsectionOperation*>(activeOperation.get()) ||
		dynamic_cast<ManipulateCapOperation*>(activeOperation.get()) ||
		dynamic_cast<ManipulateCapsOperation*>(activeOperation.get());
	}

	void onQuickSettingsChanged() const { if (activeOperation) activeOperation->onQuickSettingsChanged(); }

	[[nodiscard]] std::optional<Cursor> queryCursor() const override;

protected:
	explicit ToolMode(const EditorContext& ctx) : ctx(ctx) {}

	// Pointer down and up on the same spot
	virtual void performPrimaryAction(const PointerEvent& upEvent);
	virtual void performSecondaryAction(const PointerEvent& upEvent) {}
	[[nodiscard]] virtual std::unique_ptr<Operation> startDrag(const PointerEvent& dragStartEvent) { return nullptr; }

	const EditorContext& ctx;

	std::unique_ptr<Operation> activeOperation;

	PointerEvent pointerDownEvent;
	glm::vec2 pointer0Position{};

	[[nodiscard]] bool pointedAtBall(glm::vec2 pointerPlanarPosition) const;
	[[nodiscard]] bool pointedAtObstacle(glm::vec2 pointerPlanarPosition) const;
	[[nodiscard]] bool pointedAtEntity(glm::vec2 pointerPlanarPosition) const;

private:
	[[nodiscard]] virtual ToolModeResponse doProcessEvent(const Event& event) { return {}; }

	bool dragging = false;
	bool pointerPrimaryDown = false;
	bool pointerSecondaryDown = false;
};

#endif // TOOL_MODE_H
