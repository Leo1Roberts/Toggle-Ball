#include "editor/tool/TransformMode.h"

#include "editor/operation/RotateOperation.h"
#include "editor/operation/ScaleOperation.h"
#include "editor/operation/SelectOperation.h"
#include "editor/operation/TranslateOperation.h"


ToolModeResponse TransformMode::doProcessEvent(const Event& event) {
	if (auto key = std::get_if<KeyEvent>(&event)) {
		if (key->action == KeyAction::Down) {
			if (TranslateOperation::keyToTranslationVector(key->chord.code)) {
				activeOperation = std::make_unique<TranslateOperation>(scene, camera, settings, TriggerType::ActionKey);
				bool success =
					activeOperation->start(key->chord.modifiers) &&
					activeOperation->processEvent(event).status == OperationStatus::Running;
				if (!success)
					activeOperation.reset();
				return {.consumedEvent = true, .operationChanged = success};
			}
		}
	}

	return {.consumedEvent = false, .operationChanged = false};
}


std::unique_ptr<Operation> TransformMode::startDrag(const PointerEvent& dragStartEvent) {
	bool hit = pointedAtEntity(pointerDownEvent.position);

	if (hit) {
		auto selectOperation = SelectOperation(scene, camera, TriggerType::Pointer, camera.screenToPlanarPosition(pointerDownEvent.position), true);
		if (selectOperation.start(pointerDownEvent.modifiers))
			selectOperation.finish();
		else return nullptr;

		if (selectOperation.getMode() == SelectionMode::Subtract) {
			selectOperation.cancel();
			return nullptr;
		}
	}

	if (dragStartEvent.button == PointerButton::Primary) {
		if (hit) {
			auto translateOperation = std::make_unique<TranslateOperation>(scene, camera, settings, TriggerType::Pointer, camera.screenToPlanarPosition(pointerDownEvent.position));
			if (translateOperation->start(pointerDownEvent.modifiers))
				return translateOperation;
			return nullptr;
		}

		auto selectOperation = std::make_unique<SelectOperation>(scene, camera, TriggerType::Pointer, camera.screenToPlanarPosition(pointerDownEvent.position));
		if (selectOperation->start(pointerDownEvent.modifiers))
			return selectOperation;
		return nullptr;
	}

	if (dragStartEvent.button == PointerButton::Secondary) {
		if (hit) {
			auto rotateOperation = std::make_unique<RotateOperation>(scene, camera, settings, TriggerType::Pointer, camera.screenToPlanarPosition(pointerDownEvent.position));
			if (rotateOperation->start(pointerDownEvent.modifiers))
				return rotateOperation;
			return nullptr;
		}
	}

	return nullptr;
}