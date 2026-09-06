#ifndef DRAW_OPERATION_H
#define DRAW_OPERATION_H

#include "Operation.h"
#include "ManipulateCapOperation.h"


class DrawOperation : public Operation {
public:
	DrawOperation(const EditorContext& ctx, TriggerType trigger, glm::vec2 initialPlanarPosition, float minorRadius, std::optional<float> fixedTangentAngle = std::nullopt);

	void cancel() const final { ctx.scene.cancelLevelChange(); }
	void commit() const final { ctx.scene.commitLevelChange(); }

	void addGizmos(GizmoRenderer& gizmoRenderer) const override { manipulateCapOperation->addGizmos(gizmoRenderer); }

protected:
	[[nodiscard]] OperationResponse doProcessEvent(const Event& event) override {
		return manipulateCapOperation->doProcessEvent(event);
	}

	void applyOperation() override { manipulateCapOperation->applyOperation(); }

private:
	void applyModifiers(byte mods) final { manipulateCapOperation->applyModifiers(mods); }

	std::unique_ptr<ManipulateCapOperation> manipulateCapOperation;
};


#endif // DRAW_OPERATION_H
