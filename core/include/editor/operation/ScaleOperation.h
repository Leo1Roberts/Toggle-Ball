#ifndef SCALE_OPERATION_H
#define SCALE_OPERATION_H

#include "TransformOperation.h"


class ScaleOperation : public TransformOperation {
public:
	ScaleOperation(const EditorContext* context, TriggerType trigger, glm::vec2 initialPointerPosition = {});

	void renderGizmos() override;

private:
	bool doProcessEvent(const Event& event) override;
	void applyOperation() override;

	enum class Dimension : int { MajorAndMinor, Major, Minor, COUNT };

	glm::vec2 pointerPlanarPosition{};

	Dimension dimension = Dimension::MajorAndMinor;
	glm::vec2 pivot{};
	float scale = 1.f;
};


#endif // SCALE_OPERATION_H
