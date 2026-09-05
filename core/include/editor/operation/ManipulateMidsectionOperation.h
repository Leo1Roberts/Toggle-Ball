#ifndef CURVATURE_OPERATION_H
#define CURVATURE_OPERATION_H

#include "Operation.h"
#include "editor/EditorContext.h"
#include "editor/EditorScene.h"

class EditorObstacle;


class ManipulateMidsectionOperation : public Operation {
public:
	ManipulateMidsectionOperation(const EditorContext& ctx, TriggerType trigger, glm::vec2 initialPlanarPosition, int obstacleIndex, glm::vec2 initialHandlePosition);

	void cancel() const final { ctx.scene.cancelLevelChange(); }
	void commit() const final { ctx.scene.commitLevelChange(); }

	[[nodiscard]] std::optional<Cursor> queryCursor() const override;

protected:
	[[nodiscard]] OperationResponse doProcessEvent(const Event& event) override;

	void applyOperation() override;

private:
	void applyModifiers(byte mods) final {
		changeArcRadius = mods & MOD_SHIFT;
	}
	bool changeArcRadius = false;

	int obstacleIndex;
	EditorObstacle& obstacle;
	const ObstacleDescriptor initialDescriptor;
	const float initialAngle;
	const glm::vec2 initialPosition;
	const glm::vec2 initialHandlePosition;
	const glm::vec2 cap1, cap2;
	std::optional<float> initialHandleMinusArcRadius = std::nullopt;;
};


#endif // CURVATURE_OPERATION_H
