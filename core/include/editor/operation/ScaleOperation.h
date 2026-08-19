#ifndef SCALE_OPERATION_H
#define SCALE_OPERATION_H

#include "PivotOperation.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/norm.hpp"


class ScaleOperation : public PivotOperation {
public:
	ScaleOperation(const EditorContext* context, TriggerType trigger, glm::vec2 initialPointerPosition = {})
		: PivotOperation(context, trigger, initialPointerPosition, ActionCode::Scale) { ScaleOperation::createUI(); }
	explicit ScaleOperation(const TransformOperation& other)
		: PivotOperation(other, ActionCode::Scale) { ScaleOperation::createUI(); }

protected:
	void createUI() override;
	void updateDetailsText() override;

	bool doProcessEvent(const Event& event) override;
	void applyOperation() override;

private:
	void updateTransformation(glm::vec2 newPointerPlanarPosition) override {
		scale *= std::pow(std::sqrt(length2(newPointerPlanarPosition - pivot) / length2(pointerPlanarPosition - pivot)), precisionMultiplier);
	}

	enum class Dimension : int { MajorAndMinor, Major, Minor, COUNT };

	Dimension dimension = Dimension::MajorAndMinor;
	float scale = 1.f;
};


#endif // SCALE_OPERATION_H
