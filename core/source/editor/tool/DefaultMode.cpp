#include "editor/tool/DefaultMode.h"

#include "editor/operation/RotateOperation.h"
#include "editor/operation/SelectOperation.h"
#include "editor/operation/TranslateOperation.h"


bool DefaultMode::doProcessEvent(const Event& event) {
	if (auto key = std::get_if<KeyEvent>(&event)) {
		if (key->action == KeyAction::Down) {
			if (TranslateOperation::keyToTranslationVector(key->chord.code)) {
				auto translateOperation = std::make_unique<TranslateOperation>(context, TriggerType::ActionKey);
				if (translateOperation->start(key->chord.modifiers))
					context->loadOperation(std::move(translateOperation))->processEvent(event);
				return true;
			}
		}
	}
	return false;
}


void DefaultMode::performPrimaryAction(const PointerEvent& upEvent) {
	auto selectOperation = SelectOperation(
		context, TriggerType::Pointer,
		pointerDownEvent.position, true);
	if (selectOperation.start(pointerDownEvent.modifiers)) {
		selectOperation.finish();
		selectOperation.commit();
	}
}

void DefaultMode::startDrag(const PointerEvent& dragStartEvent) {
	bool hit = false;
	auto hitTestBox = SelectBox(context->camera->screenToPlanarPosition(pointerDownEvent.position));
	if (context->scene->ball.isInSelectBox(hitTestBox))
		hit = true;
	else
		for (const auto& obstacle : context->scene->obstacles)
			if (obstacle.isInSelectBox(hitTestBox)) {
				hit = true;
				break;
			}

	if (hit) {
		auto selectOperation = (SelectOperation*)context->loadOperation(std::make_unique<SelectOperation>(
			context, TriggerType::Pointer,
			pointerDownEvent.position, true));
		if (selectOperation->start(pointerDownEvent.modifiers)) {
			selectOperation->finish();
			context->unloadOperation();
		}
		else return;

		if (selectOperation->getMode() == SelectionMode::Subtract) {
			selectOperation->cancel();
			return;
		}
	}

	if (dragStartEvent.button == PointerButton::Primary) {
		if (hit) {
			auto translateOperation = std::make_unique<TranslateOperation>(
				context, TriggerType::Pointer,
				pointerDownEvent.position);
			if (translateOperation->start(pointerDownEvent.modifiers))
				context->loadOperation(std::move(translateOperation));
		} else {
			auto selectOperation = std::make_unique<SelectOperation>(
				context, TriggerType::Pointer,
				pointerDownEvent.position);
			if (selectOperation->start(pointerDownEvent.modifiers))
				context->loadOperation(std::move(selectOperation));
		}
	} else if (dragStartEvent.button == PointerButton::Secondary) {
		if (hit) {
			auto rotateOperation = std::make_unique<RotateOperation>(
				context, TriggerType::Pointer,
				pointerDownEvent.position);
			if (rotateOperation->start(pointerDownEvent.modifiers))
				context->loadOperation(std::move(rotateOperation));
		}
	}
}