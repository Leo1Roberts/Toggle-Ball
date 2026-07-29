#include "DefaultMode.h"

#include "SelectOperation.h"


void DefaultMode::performPrimaryAction(const PointerEvent& upEvent) {
	auto selectOperation = SelectOperation(
		context, TriggerType::PointerPrimary,
		pointerDownEvent.position, pointerDownEvent.modifiers, true);

	selectOperation.finish();
	selectOperation.commit();
}


Operation* DefaultMode::startPrimaryDragOperation() {
	auto selectOperation = std::make_unique<SelectOperation>(
		context, TriggerType::PointerPrimary,
		pointerDownEvent.position, pointerDownEvent.modifiers);

	return context->startOperation(std::move(selectOperation));
}