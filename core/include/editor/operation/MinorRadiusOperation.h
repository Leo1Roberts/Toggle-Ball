#ifndef MINOR_RADIUS_OPERATION_H
#define MINOR_RADIUS_OPERATION_H

#include "Operation.h"
#include "editor/EditorScene.h"


class MinorRadiusOperation : public Operation {
public:
	MinorRadiusOperation(EditorScene* scene, const Camera* camera, TriggerType trigger, glm::vec2 initialPlanarPosition)
		: Operation(scene, camera, trigger, initialPlanarPosition) {
		initialDistance = scene->obstacles[scene->selectionFocus.index].getRimProximity(initialPlanarPosition).distance;
	}

	[[nodiscard]] float getFinalRadius() const { return finalRadius; } // Only call after finish()

	void finish() override {
		finalRadius = scene->obstacles[scene->selectionFocus.index].descriptor->shape->minorRadius;
	}

	void cancel() const override { scene->cancelLevelChange(); }
	void commit() const override { scene->commitLevelChange(); }

protected:
	OperationResponse doProcessEvent(const Event& event) override;
	void applyOperation() override;

private:
	void applyModifiers(byte mods) final {}

	float initialDistance;
	float adjustment = 0.f;
	float finalRadius{};
};


#endif // MINOR_RADIUS_OPERATION_H
