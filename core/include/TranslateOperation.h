#ifndef TRANSLATE_OPERATION_H
#define TRANSLATE_OPERATION_H

#include "MotionOperation.h"
#include "Operation.h"


class TranslateOperation : public MotionOperation {
public:
	TranslateOperation(const EditorContext* context, TriggerType trigger, glm::vec2 initialPointerPosition)
		: MotionOperation(context, trigger, initialPointerPosition) {}

private:
	bool doProcessEvent(const Event& event) override;
	void applyOperation() override;

	bool lockToAxis = false;
	glm::vec2 baseAxis{};
	bool localRead = false;
	bool localWrite = false;
	glm::vec2 rawTranslation{0.f};
};


#endif // TRANSLATE_OPERATION_H
