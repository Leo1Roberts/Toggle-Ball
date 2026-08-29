#ifndef MANIPULATE_CAP_OPERATION_H
#define MANIPULATE_CAP_OPERATION_H

#include "Operation.h"
#include "editor/EditorScene.h"


class ManipulateCapOperation : public Operation {
	friend class DrawOperation;
public:
	ManipulateCapOperation(EditorScene& scene, const Camera& camera, TriggerType trigger, glm::vec2 initialPlanarPosition, EditorObstacle& obstacle, bool leftCap, float tangentAngle = NAN);

	// DrawOperation relies on cancel/commitLevelChange being called
	void cancel() const final { scene.cancelLevelChange(); }
	void commit() const final { scene.commitLevelChange(); }

protected:
	[[nodiscard]] OperationResponse doProcessEvent(const Event& event) override;

	void applyOperation() override;

private:
	void applyModifiers(byte mods) final {
		alignWithTangent = mods & MOD_CTRL && !std::isnan(tangentAngle);
	}
	bool alignWithTangent = false;

	EditorObstacle& obstacle;
	const ObstacleDescriptor initialDescriptor;
	const float initialAngle;
	const glm::vec2 initialPosition;
	const bool leftCap;
	const float tangentAngle;
	const glm::vec2 fixedCapPlanarPosition;
	const glm::vec2 initialCapPlanarPosition;
};

class ManipulateLeftCapOperation : public ManipulateCapOperation {
public:
	ManipulateLeftCapOperation(EditorScene& scene, const Camera& camera, TriggerType trigger, glm::vec2 initialPlanarPosition, EditorObstacle& obstacle, float tangentAngle = NAN)
		: ManipulateCapOperation(scene, camera, trigger, initialPlanarPosition, obstacle, true, tangentAngle) {}
};
class ManipulateRightCapOperation : public ManipulateCapOperation {
public:
	ManipulateRightCapOperation(EditorScene& scene, const Camera& camera, TriggerType trigger, glm::vec2 initialPlanarPosition, EditorObstacle& obstacle, float tangentAngle = NAN)
		: ManipulateCapOperation(scene, camera, trigger, initialPlanarPosition, obstacle, false, tangentAngle) {}
};


#endif // MANIPULATE_CAP_OPERATION_H
