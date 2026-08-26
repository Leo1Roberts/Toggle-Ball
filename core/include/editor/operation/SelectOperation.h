#ifndef SELECT_OPERATION_H
#define SELECT_OPERATION_H

#include "editor/EditorScene.h"
#include "Operation.h"


enum class SelectionMode { Replace, Add, Subtract };

class SelectOperation : public Operation {
public:
	SelectOperation(EditorScene& scene, const Camera& camera, TriggerType trigger, glm::vec2 initialPlanarPosition, bool instant = false)
		: Operation(scene, camera, trigger, initialPlanarPosition), box(initialPlanarPosition), instant(instant) {}

	[[nodiscard]] std::vector<BindingHint> getBindingHints() const override;
	void renderGizmos(GizmoRenderer& gizmoRenderer) final;

	void finish() final;

	void cancel() const final { return scene.cancelSelectionChange(); }
	void commit() const final { return scene.commitSelectionChange(); }

	[[nodiscard]] SelectionMode getMode() const { return mode; }

protected:
	OperationResponse doProcessEvent(const Event& event) final;
	void applyOperation() final;

private:
	void applyModifiers(byte mods) final;

	SelectionMode mode = SelectionMode::Replace;
	SelectBox box{};
	bool instant;
};


#endif // SELECT_OPERATION_H
