#include "Operation.h"


void Operation::processEvent(const Event& event) {
	if (auto* key = std::get_if<KeyEvent>(&event)) {
		if (key->action == KeyAction::Down) {
			if (key->chord.code == KeyCode::Escape) {
				cancel();
				return;
			}
			if (key->chord.code == KeyCode::Enter) {
				finish();
				commit();
				return;
			}
		}
		switch (key->chord.code) {
		case KeyCode::Ctrl:
		case KeyCode::Shift:
		case KeyCode::Alt:
			applyModifiers(key->chord.modifiers);
			applyOperation();
		default:;
		}
	} else if (auto* pointer = std::get_if<PointerEvent>(&event)) {
		if (pointer->action == PointerAction::Down) {
			if ((pointer->button == PointerButton::Primary && trigger == TriggerType::PointerSecondary) ||
				(pointer->button == PointerButton::Secondary && trigger == TriggerType::PointerPrimary)) {
				cancel();
				return;
			}
			if (trigger == TriggerType::Key) {
				if (pointer->button == PointerButton::Primary) {
					finish();
					commit();
					return;
				}
				if (pointer->button == PointerButton::Secondary) {
					cancel();
					return;
				}
			}
		} else if (pointer->action == PointerAction::Up) {
			if ((pointer->button == PointerButton::Primary && trigger == TriggerType::PointerPrimary) ||
				(pointer->button == PointerButton::Secondary && trigger == TriggerType::PointerSecondary)) {
				finish();
				commit();
				return;
			}
		}
	}

	doProcessEvent(event);
}