#ifndef MANIPULATE_CAP_OPERATION_H
#define MANIPULATE_CAP_OPERATION_H

#include "Operation.h"
#include "editor/EditorScene.h"


class ManipulateCapOperation : public Operation {
	friend class DrawOperation;
public:
	ManipulateCapOperation(EditorScene& scene, const Camera& camera, TriggerType trigger, glm::vec2 initialPlanarPosition, EditorObstacle& obstacle, bool leftCap, glm::vec2 tangent = glm::vec2(0.f));

	// DrawOperation relies on cancel/commitLevelChange being called
	void cancel() const final { scene.cancelLevelChange(); }
	void commit() const final { scene.commitLevelChange(); }

protected:
	[[nodiscard]] OperationResponse doProcessEvent(const Event& event) override;

	void applyOperation() override;

private:
	void applyModifiers(byte mods) final {
		alignWithTangent = mods & MOD_CTRL && tangent != glm::vec2(0.f);
	}
	bool alignWithTangent = false;

	ObstacleDescriptor initialDescriptor;
	EditorObstacle& obstacle;
	bool leftCap;
	glm::vec2 tangent;
	glm::vec2 fixedCapPlanarPosition;
	glm::vec2 initialCapPlanarPosition;
};


#endif // MANIPULATE_CAP_OPERATION_H
