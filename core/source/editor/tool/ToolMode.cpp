#include "editor/tool/ToolMode.h"


ToolModeResponse ToolMode::processEvent(const Event& event) {
	bool operationChanged = false;

	if (activeOperation) {
		auto response = activeOperation->processEvent(event);
		if (response.status != OperationStatus::Running) {
			activeOperation.reset();
			operationChanged = true;
		}
		if (response.consumedEvent)
			return {.consumedEvent = true, .operationChanged = operationChanged};
	}

	if (auto* pointer = std::get_if<PointerEvent>(&event)) {
		switch (pointer->action) {
		case PointerAction::Down:
			pointerDownEvent = *pointer;
			if (pointer->button == PointerButton::Primary)
				pointerPrimaryDown = true;
			else if (pointer->button == PointerButton::Secondary)
				pointerSecondaryDown = true;
			break;
		case PointerAction::StartDrag:
			dragging = true;
			if (pointer->button == PointerButton::Primary && pointerPrimaryDown ||
				pointer->button == PointerButton::Secondary && pointerSecondaryDown) {
				activeOperation = startDrag(*pointer);
				return {.consumedEvent = true, .operationChanged = activeOperation != nullptr};
			}
			return {.consumedEvent = true, .operationChanged = false};
		case PointerAction::Up:
			if (dragging) {
				if (pointer->button == PointerButton::Primary)
					pointerPrimaryDown = false;
				else if (pointer->button == PointerButton::Secondary)
					pointerSecondaryDown = false;
				if (!pointerPrimaryDown && !pointerSecondaryDown)
					dragging = false;
			} else {
				if (pointer->button == PointerButton::Primary) {
					bool doPrimaryAction = pointerPrimaryDown && !pointerSecondaryDown;
					pointerPrimaryDown = false;
					if (doPrimaryAction) {
						performPrimaryAction(*pointer);
						return {.consumedEvent = true, .operationChanged = false};
					}
				} else if (pointer->button == PointerButton::Secondary) {
					bool doSecondaryAction = pointerSecondaryDown && !pointerPrimaryDown;
					pointerSecondaryDown = false;
					if (doSecondaryAction) {
						performSecondaryAction(*pointer);
						return {.consumedEvent = true, .operationChanged = false};
					}
				}
			}
			break;
		default:;
		}
	}

	auto response = doProcessEvent(event);
	response.operationChanged |= operationChanged;
	return response;
}


void ToolMode::createOperationUI(UINode& container) const {
	if (activeOperation) {
		activeOperation->createUI(container);
		activeOperation->updateUI();
	}
}


void ToolMode::cancelActiveOperation() {
	if (activeOperation)
		activeOperation->cancel();
	activeOperation.reset();
}
void ToolMode::commitActiveOperation() {
	if (activeOperation) {
		activeOperation->finish();
		activeOperation->commit();
	}
	activeOperation.reset();
}