#include "DefaultMode.h"

#include "SelectOperation.h"
#include "TranslateOperation.h"


void DefaultMode::performPrimaryAction(const PointerEvent& upEvent) {
	auto selectOperation = SelectOperation(
		context, TriggerType::PointerPrimary,
		pointerDownEvent.position, true);
	if (selectOperation.start(pointerDownEvent.modifiers)) {
		selectOperation.finish();
		selectOperation.commit();
	}
}


Operation* DefaultMode::startPrimaryDragOperation() {
	bool hit = false;
	auto hitTestBox = SelectBox(context->camera->screenToPlanarPosition(pointerDownEvent.position));
	if (context->scene->getBall()->isInSelectBox(hitTestBox))
		hit = true;
	else {
		for (const auto& obstacle : context->scene->getObstacles())
			if (obstacle.isInSelectBox(hitTestBox)) {
				hit = true;
				break;
			}
	}

	if (hit) {
		auto selectOperation = SelectOperation(
			context, TriggerType::PointerPrimary,
			pointerDownEvent.position, true);
		if (selectOperation.start(pointerDownEvent.modifiers))
			selectOperation.finish();
		else return nullptr;

		if (selectOperation.getMode() == SelectionMode::Subtract) {
			selectOperation.cancel();
			return nullptr;
		}

		auto translateOperation = std::make_unique<TranslateOperation>(
			context, TriggerType::PointerPrimary,
			pointerDownEvent.position);
		if (translateOperation->start(pointerDownEvent.modifiers))
			return context->startOperation(std::move(translateOperation));
		return nullptr;
	}

	auto selectOperation = std::make_unique<SelectOperation>(
		context, TriggerType::PointerPrimary,
		pointerDownEvent.position);
	if (selectOperation->start(pointerDownEvent.modifiers))
		return context->startOperation(std::move(selectOperation));
	return nullptr;
}