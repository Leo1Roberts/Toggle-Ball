#ifndef MANIPULATE_CAP_OPERATION_H
#define MANIPULATE_CAP_OPERATION_H

#include "Operation.h"
#include "editor/EditorContext.h"
#include "editor/EditorScene.h"


class ManipulateCapOperation : public Operation {
	friend class DrawOperation;
public:
	ManipulateCapOperation(const EditorContext& ctx, TriggerType trigger, glm::vec2 initialPlanarPosition, int obstacleIndex, bool leftCap, std::optional<float> tangentAngle = std::nullopt);

	// DrawOperation relies on cancel/commitLevelChange being called
	void cancel() const final { ctx.scene.cancelLevelChange(); }
	void commit() const final { ctx.scene.commitLevelChange(); }

	void addGizmos(GizmoRenderer& gizmoRenderer) const override;

protected:
	[[nodiscard]] OperationResponse doProcessEvent(const Event& event) override;

	void applyOperation() override;

private:
	void applyModifiers(byte mods) final {
		alignWithTangent = mods & MOD_CTRL && tangentAngle;
	}
	bool alignWithTangent = false;

	int obstacleIndex;
	EditorObstacle& obstacle;
	const ObstacleDescriptor initialDescriptor;
	const float initialAngle;
	const glm::vec2 initialPosition;
	const bool leftCap;
	const std::optional<float> tangentAngle;
	const glm::vec2 fixedCapPlanarPosition;
	const glm::vec2 initialCapPlanarPosition;

	bool currentlyLeftCap = leftCap;
	SnapResult snapResult;
};

class ManipulateLeftCapOperation : public ManipulateCapOperation {
public:
	ManipulateLeftCapOperation(const EditorContext& ctx, TriggerType trigger, glm::vec2 initialPlanarPosition, int obstacleIndex, std::optional<float> tangentAngle = std::nullopt)
		: ManipulateCapOperation(ctx, trigger, initialPlanarPosition, obstacleIndex, true, tangentAngle) {}
};
class ManipulateRightCapOperation : public ManipulateCapOperation {
public:
	ManipulateRightCapOperation(const EditorContext& ctx, TriggerType trigger, glm::vec2 initialPlanarPosition, int obstacleIndex, std::optional<float> tangentAngle = std::nullopt)
		: ManipulateCapOperation(ctx, trigger, initialPlanarPosition, obstacleIndex, false, tangentAngle) {}
};


#endif // MANIPULATE_CAP_OPERATION_H
