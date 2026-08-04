#include "io/Event.h"
#include "io/Input.h"


void InputState::updateFromEvent(const PointerEvent& event) {
	PointerState& state = pointers[event.id];
	state.position = event.position;

	if (event.action == PointerAction::Down)
		state.activeButtons.insert(event.button);
	else if (event.action == PointerAction::Up) {
		state.activeButtons.erase(event.button);

		if (event.id > 0 && state.activeButtons.empty())
			pointers.erase(event.id);
	}
}