#ifndef DRAW_OPERATION_H
#define DRAW_OPERATION_H

#include "ManipulateCapOperation.h"
#include "editor/operation/Operation.h"


class DrawOperation : public Operation {
public:
	DrawOperation(EditorScene& scene, const Camera& camera, const EditorQuickSettings& quickSettings, TriggerType trigger, glm::vec2 initialPlanarPosition, float minorRadius, std::optional<float> tangentAngle = std::nullopt);

	// Relies on manipulateCapOperation calling cancel/commitLevelChange
	void cancel() const final { manipulateCapOperation->cancel(); }
	void commit() const final { manipulateCapOperation->commit(); }

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
