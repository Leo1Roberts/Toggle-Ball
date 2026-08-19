#include "editor/operation/Operation.h"


OperationResponse Operation::processEvent(const Event& event) {
	if (auto* key = std::get_if<KeyEvent>(&event)) {
		if (key->action == KeyAction::Down) {
			if (key->chord.code == KeyCode::Escape) {
				cancel();
				return {true, OperationStatus::Cancelled};
			}
			if (key->chord.code == KeyCode::Enter) {
				finish();
				commit();
				return {true, OperationStatus::Committed};
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
		switch (pointer->action) {
		case PointerAction::FinishDrag:
			finish();
			commit();
			return {true, OperationStatus::Committed};
		case PointerAction::CancelDrag:
			cancel();
			return {true, OperationStatus::Cancelled};
		case PointerAction::Down:
			if (trigger == TriggerType::TriggerKey || trigger == TriggerType::ActionKey) {
				if (pointer->button == PointerButton::Primary) {
					finish();
					commit();
					return {trigger == TriggerType::TriggerKey, OperationStatus::Committed};
				}
				if (pointer->button == PointerButton::Secondary) {
					cancel();
					return {trigger == TriggerType::TriggerKey, OperationStatus::Cancelled};
				}
			}
			break;
		default:;
		}
	}

	return doProcessEvent(event);
}