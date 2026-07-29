#include "ToolMode.h"


bool ToolMode::processEvent(const Event& event) {
	if (auto* pointer = std::get_if<PointerEvent>(&event)) {
		if (pointer->action == PointerAction::Down) {
			pointerDownEvent = *pointer;
			if (pointer->button == PointerButton::Primary)
				pointerPrimaryDown = true;
			else if (pointer->button == PointerButton::Secondary)
				pointerSecondaryDown = true;
		} else if (pointer->action == PointerAction::Move && (pointerPrimaryDown != pointerSecondaryDown)) {
			if (pointerPrimaryDown) {
				startPrimaryDrag(*pointer);
				pointerPrimaryDown = false;
			} else if (pointerSecondaryDown) {
				startSecondaryDrag(*pointer);
				pointerSecondaryDown = false;
			}
		} else if (pointer->action == PointerAction::Up) {
			if (pointer->button == PointerButton::Primary) {
				if (pointerPrimaryDown && !pointerSecondaryDown)
					performPrimaryAction(*pointer);
				pointerPrimaryDown = false;
			} else if (pointer->button == PointerButton::Secondary) {
				if (pointerSecondaryDown && !pointerPrimaryDown)
					performSecondaryAction(*pointer);
				pointerSecondaryDown = false;
			}
		}
	}

	return doProcessEvent(event);
}