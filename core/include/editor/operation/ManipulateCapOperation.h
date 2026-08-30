#ifndef MANIPULATE_CAP_OPERATION_H
#define MANIPULATE_CAP_OPERATION_H

#include "Operation.h"
#include "editor/EditorScene.h"


class ManipulateCapOperation : public Operation {
	friend class DrawOperation;
public:
	ManipulateCapOperation(EditorScene& scene, const Camera& camera, const EditorQuickSettings& quickSettings, TriggerType trigger, glm::vec2 initialPlanarPosition, EditorObstacle& obstacle, bool leftCap, std::optional<float> tangentAngle = std::nullopt);

	// DrawOperation relies on cancel/commitLevelChange being called
	void cancel() const final { scene.cancelLevelChange(); }
	void commit() const final { scene.commitLevelChange(); }

protected:
	[[nodiscard]] OperationResponse doProcessEvent(const Event& event) override;

	void applyOperation() override;

private:
	void applyModifiers(byte mods) final {
		alignWithTangent = mods & MOD_CTRL && tangentAngle;
	}
	bool alignWithTangent = false;

	EditorObstacle& obstacle;
	const ObstacleDescriptor initialDescriptor;
	const float initialAngle;
	const glm::vec2 initialPosition;
	const bool leftCap;
	std::optional<float> tangentAngle;
	const glm::vec2 fixedCapPlanarPosition;
	const glm::vec2 initialCapPlanarPosition;
	glm::vec2 capPlanarPosition;
};

class ManipulateLeftCapOperation : public ManipulateCapOperation {
public:
	ManipulateLeftCapOperation(EditorScene& scene, const Camera& camera, const EditorQuickSettings& quickSettings, TriggerType trigger, glm::vec2 initialPlanarPosition, EditorObstacle& obstacle, std::optional<float> tangentAngle = std::nullopt)
		: ManipulateCapOperation(scene, camera, quickSettings, trigger, initialPlanarPosition, obstacle, true, tangentAngle) {}
};
class ManipulateRightCapOperation : public ManipulateCapOperation {
public:
	ManipulateRightCapOperation(EditorScene& scene, const Camera& camera, const EditorQuickSettings& quickSettings, TriggerType trigger, glm::vec2 initialPlanarPosition, EditorObstacle& obstacle, std::optional<float> tangentAngle = std::nullopt)
		: ManipulateCapOperation(scene, camera, quickSettings, trigger, initialPlanarPosition, obstacle, false, tangentAngle) {}
};


#endif // MANIPULATE_CAP_OPERATION_H
