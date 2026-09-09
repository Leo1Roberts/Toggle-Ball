#ifndef MANIPULATE_CAPS_OPERATION_H
#define MANIPULATE_CAPS_OPERATION_H

#include "Operation.h"
#include "ManipulateCapOperation.h"


class ManipulateCapsOperation : public Operation {
public:
	ManipulateCapsOperation(const EditorContext& ctx, TriggerType trigger, glm::vec2 initialPlanarPosition, const std::vector<CapInfo>& allCapsInfo);

	void cancel() const final { ctx.scene.cancelLevelChange(); }
	void commit() const final { ctx.scene.commitLevelChange(); }

	void addGizmos(GizmoRenderer& gizmoRenderer) const override;

protected:
	[[nodiscard]] OperationResponse doProcessEvent(const Event& event) override;

	void applyOperation() override;

private:
	void applyModifiers(byte mods) final;
	bool preserveLinkedAngles = false;
	bool smoothJoin = false;

	std::vector<EntityReference> manipulatedEntities;
	SnapResult snapResult;

	std::vector<ManipulateCapOperation> manipulateCapOperations;
};


#endif // MANIPULATE_CAPS_OPERATION_H
