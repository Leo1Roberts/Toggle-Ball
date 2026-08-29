#ifndef MINOR_RADIUS_OPERATION_H
#define MINOR_RADIUS_OPERATION_H

#include "Operation.h"
#include "editor/EditorScene.h"


class MinorRadiusOperation : public Operation {
public:
	MinorRadiusOperation(EditorScene& scene, const Camera& camera, TriggerType trigger, glm::vec2 initialPlanarPosition, float& minorRadius) :
		Operation(scene, camera, trigger, initialPlanarPosition), minorRadius(minorRadius),
		initialDistance(scene.obstacles[scene.selectionFocus.index].getRimProximity(initialPlanarPosition).distance) {}

	void cancel() const override { scene.cancelLevelChange(); }
	void commit() const override {
		scene.commitLevelChange();
		minorRadius = scene.obstacles[scene.selectionFocus.index].descriptor->shape->minorRadius;
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
