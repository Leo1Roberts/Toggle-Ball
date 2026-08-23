#ifndef DRAW_OPERATION_H
#define DRAW_OPERATION_H

#include "editor/EditorScene.h"
#include "editor/operation/Operation.h"


class DrawOperation : public Operation {
public:
	DrawOperation(EditorScene* scene, const Camera* camera, TriggerType trigger, glm::vec2 initialPlanarPosition, float minorRadius, glm::vec2 tangent = glm::vec2(0.f));

	void cancel() const final { return scene->cancelLevelChange(); }
	void commit() const final { return scene->commitLevelChange(); }

protected:
	[[nodiscard]] OperationResponse doProcessEvent(const Event& event) override;

	void applyOperation() override;

private:
	void applyModifiers(byte mods) final {}

	float minorRadius;
	glm::vec2 tangent;

	EditorObstacle* obstacle;

	glm::vec2 terminalPlanarPosition;
};


#endif // DRAW_OPERATION_H
