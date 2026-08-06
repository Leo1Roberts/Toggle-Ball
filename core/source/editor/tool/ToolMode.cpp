#include "editor/tool/ToolMode.h"


bool ToolMode::processEvent(const Event& event) {
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
				startDrag(*pointer);
			}
			return true;
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
					if (pointerPrimaryDown && !pointerSecondaryDown)
						performPrimaryAction(*pointer);
					pointerPrimaryDown = false;
				} else if (pointer->button == PointerButton::Secondary) {
					if (pointerSecondaryDown && !pointerPrimaryDown)
						performSecondaryAction(*pointer);
					pointerSecondaryDown = false;
				}
			}
			break;
		default:;
		}
	}

	return doProcessEvent(event);
}