#ifndef TRANSLATE_OPERATION_H
#define TRANSLATE_OPERATION_H

#include "TransformOperation.h"
#include "Operation.h"


class TranslateOperation : public TransformOperation {
public:
	TranslateOperation(const EditorContext* context, TriggerType trigger, glm::vec2 initialPointerPosition)
		: TransformOperation(context, trigger, initialPointerPosition) {}

private:
	void doProcessEvent(const Event& event) override;
	void applyOperation() override;

	void setMode(glm::vec2 requestedAxis);

	bool lockToAxis = false;
	glm::vec2 baseAxis{};
	bool localRead = false;
	glm::vec2 rawTranslation{0.f};
};


#endif // TRANSLATE_OPERATION_H
