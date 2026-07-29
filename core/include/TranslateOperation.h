#ifndef TRANSLATE_OPERATION_H
#define TRANSLATE_OPERATION_H

#include "EditorScene.h"
#include "MotionOperation.h"
#include "Operation.h"


class MotionOperation;
enum class AxisLock { None, GlobalX, GlobalY, LocalX, LocalY };

class TranslateOperation : public MotionOperation {
public:
	TranslateOperation(const EditorContext* context, TriggerType trigger, glm::vec2 initialPointerPosition, byte mods)
		: MotionOperation(context, trigger, initialPointerPosition, mods) {}

private:
	bool doProcessEvent(const Event& event) override;
	void applyOperation() override;

	AxisLock axisLock = AxisLock::None;
};


#endif // TRANSLATE_OPERATION_H
