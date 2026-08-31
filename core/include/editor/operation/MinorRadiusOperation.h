#ifndef MINOR_RADIUS_OPERATION_H
#define MINOR_RADIUS_OPERATION_H

#include "Operation.h"
#include "editor/EditorContext.h"
#include "editor/EditorScene.h"


class MinorRadiusOperation : public Operation {
public:
	MinorRadiusOperation(const EditorContext& ctx, TriggerType trigger, glm::vec2 initialPlanarPosition, float& minorRadius);

	void cancel() const override { ctx.scene.cancelLevelChange(); }
	void commit() const override {
		ctx.scene.commitLevelChange();
		minorRadius = ctx.scene.obstacles[ctx.scene.selectionFocus.index].descriptor->shape->minorRadius;
	}

	[[nodiscard]] std::optional<Cursor> queryCursor() const override;

protected:
	OperationResponse doProcessEvent(const Event& event) override;
	void applyOperation() override;

private:
	void applyModifiers(byte mods) final {}

	float& minorRadius;

	const float initialDistance;
	float adjustment = 0.f;
};


#endif // MINOR_RADIUS_OPERATION_H
