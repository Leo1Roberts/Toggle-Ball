#ifndef DRAW_OPERATION_H
#define DRAW_OPERATION_H

#include "ManipulateCapOperation.h"
#include "editor/operation/Operation.h"


class DrawOperation : public Operation {
public:
	DrawOperation(const EditorContext& ctx, TriggerType trigger, glm::vec2 initialPlanarPosition, float minorRadius, std::optional<float> fixedTangentAngle = std::nullopt);

	// Relies on manipulateCapOperation calling cancel/commitLevelChange
	void cancel() const final { manipulateCapOperation->cancel(); }
	void commit() const final { manipulateCapOperation->commit(); }

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
