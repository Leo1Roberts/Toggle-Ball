#include "ui/UIDialogue.h"

#include "ui/Theme.h"


UIDialogue::UIDialogue() {
	panelStyle = { .fillColor = {Color::Black, 0.2f} };
	dialogue = addChild<DialogueBox>(Theme::DarkCard);
	dialogue->setLayout({
		.anchor = Anchor::Centre,
		.widthMode  = SizingMode::Absolute, .width  = 400.f,
		.heightMode = SizingMode::Absolute, .height = 300.f,
		.padding = {15.f, 20.f}
	});

	initialised = true;
}


UIResponse UIDialogue::interceptFocusedChildEvent(const Event& event) {
	if (auto key = std::get_if<KeyEvent>(&event)) {
		if (key->action == KeyAction::Down) {
			if (key->chord.code == KeyCode::Escape) {
				if (onReturnCallback)
					onReturnCallback();
				return UIResponse::Consumed;
			}
			if (key->chord.code == KeyCode::Enter) {
				if (onConfirmCallback)
					onConfirmCallback();
				return UIResponse::Consumed;
			}
		}
	}
	return UIResponse::Ignored;
}
UIResponse UIDialogue::processEvent(const Event& event) {
	auto response = interceptFocusedChildEvent(event);
	if (response != UIResponse::Ignored)
		return response;

	if (auto* pointer = std::get_if<PointerEvent>(&event)) {
		if (pointer->action == PointerAction::Move || pointer->action == PointerAction::Drag)
			return UIResponse::Ignored;

		if (pointer->button == PointerButton::Primary) {
			switch (pointer->action) {
			case PointerAction::Down:
				pressed = true;
				break;
			case PointerAction::Up:
				if (pressed) {
					pressed = false;
					if (onReturnCallback)
						onReturnCallback();
				}
				break;
			default:;
			}
		}
	}

	return UIResponse::Consumed;
}


void UIDialogue::setLayout(Layout l) {
	l.anchor = Anchor::Centre;
	dialogue->setLayout(l);
}