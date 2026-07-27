#include "ToolMode.h"


bool ToolMode::processEvent(const Event& event, EditorContext& editor) {
	if (auto* pointer = std::get_if<PointerEvent>(&event)) {
		if (pointer->action == PointerAction::Down) {
			pointerDownPosition = pointer->position;
			if (pointer->button == PointerButton::Primary)
				pointerPrimaryDown = true;
			else if (pointer->button == PointerButton::Secondary)
				pointerSecondaryDown = true;
		} else if (pointer->action == PointerAction::Move && (pointerPrimaryDown != pointerSecondaryDown)) {
			if (pointerPrimaryDown)
				startPrimaryDrag(editor, pointerDownPosition, *pointer);
			else if (pointerSecondaryDown)
				startSecondaryDrag(editor, pointerDownPosition, *pointer);
		} else if (pointer->action == PointerAction::Up && (pointerPrimaryDown != pointerSecondaryDown)) {
			if (pointerPrimaryDown && pointer->button == PointerButton::Primary) {
				performPrimaryAction(editor, *pointer);
				pointerPrimaryDown = false;
			} else if (pointerPrimaryDown && pointer->button == PointerButton::Secondary) {
				performSecondaryAction(editor, *pointer);
				pointerSecondaryDown = false;
			}
		}
	}

	return false;
}