#ifndef SCALE_OPERATION_H
#define SCALE_OPERATION_H

#include "PivotOperation.h"


class ScaleOperation : public PivotOperation {
public:
	ScaleOperation(const EditorContext* context, TriggerType trigger, glm::vec2 initialPointerPosition = {})
		: PivotOperation(context, trigger, initialPointerPosition) { ScaleOperation::updateDetailsText(); }
	explicit ScaleOperation(const TransformOperation& other)
		: PivotOperation(other) { ScaleOperation::updateDetailsText(); }

protected:
	void updateDetailsText() override;

private:
	bool doProcessEvent(const Event& event) override;
	void applyOperation() override;

	enum class Dimension : int { MajorAndMinor, Major, Minor, COUNT };

	Dimension dimension = Dimension::MajorAndMinor;
	float scale = 1.f;
};


#endif // SCALE_OPERATION_H
